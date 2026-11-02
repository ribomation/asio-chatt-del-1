# asio-chatt-del-1

Demoprogram till artikeln **[Bygg en chattserver med Boost.Asio, del 1: Grunderna och synkron I/O](https://www.ribomation.se/blog/2026/bygg-en-chattserver-med-boost-asio-del-1/)**
på [ribomation.se](https://www.ribomation.se/blog/).

Artikeln är den första av fem om Boost.Asio. Genom serien byggs en chattserver som till slut har
TLS, UDP-discovery och ett admin-UI i webbläsaren. Den här delen handlar om den mentala modellen
för Asio och om **synkron** I/O: vad man får, och var det tar stopp.

| Del | Innehåll | Repo |
|---|---|---|
| 1 | Grunderna och synkron I/O | detta repo |
| 2 | Asynkront med callbacks | `asio-chatt-del-2` |
| 3 | Asynkront med coroutines | `asio-chatt-del-3` |
| 4 | TLS, UDP och graceful shutdown | `asio-chatt-del-4` |
| 5 | Admin-UI med Boost.Beast och WebSocket | `asio-chatt-del-5` |

## Programmen

| Mapp | Program | Visar |
|---|---|---|
| [echoserver/](echoserver/) | `echoserver` | `tcp::acceptor`, `accept()`, `read_some` i loop, `asio::write`. En klient i taget. |
| [daytime/](daytime/) | `daytime-server`, `daytime-client` | Server med egen klient: `tcp::resolver`, `asio::connect` över endpoint-listan, `asio::read` till eof. |
| [chatserver-single-threaded/](chatserver-single-threaded/) | `chatserver-single-threaded` | Chatt steg 1a: radläsning med `read_until` + `dynamic_buffer`, eko med prefix `[du]`. En andra klient hänger i accept-kön. |
| [chatserver-multi-threaded/](chatserver-multi-threaded/) | `chatserver-multi-threaded` | Chatt steg 1b: läs-tråd per klient, en broadcast-tråd, kö med `stop_token`, sessioner via `shared_ptr`. Fungerar, men kostar. |
| [client-simulator/](client-simulator/) | `chat-sim.py` | Quick & dirty simulator i Python som startar N klienter mot chattservern. |

Felhantering i alla program: exceptions som default (try/catch i `main`), `error_code` där fel är
normalt flöde, t.ex. eof när klienten kopplar ned.

## Förutsättningar

- Boost 1.92 (Asio är header-only, men länkar mot pthreads)
- GCC 16 och C++26 (`std::println`, `std::format`, `std::chrono::zoned_time`)
- CMake 3.28 eller senare, Ninja
- `nc` (netcat) som klient, Python 3 för simulatorn

CMake-konfigurationen i [cmake/common.cmake](cmake/common.cmake) hittar Boost via handskrivna
find-moduler i `~/Libs/CMake-modules` (se [Kom igång med Boost](https://www.ribomation.se/blog/2026/kom-igang-med-boost/)).
Har du Boost någon annanstans räcker det att byta ut raden `list(APPEND CMAKE_MODULE_PATH ...)`
mot `-DBoost_ROOT=/sökväg/till/boost`, eller låta `find_package(Boost CONFIG)` hitta en systeminstallation.

## Bygga

Alla program från projektroten, med presets (`debug` eller `release`):

```sh
cmake --preset release
cmake --build --preset release
```

Binärerna hamnar i `build/release/<mapp>/`. Ett program i taget, från dess egen mapp:

```sh
cd echoserver
cmake -S . -B build -G Ninja
cmake --build build
./build/echoserver
```

## Köra

Alla servrar tar porten som valfritt första argument.

| Program | Default port | Klient |
|---|---|---|
| `echoserver` | 7000 | `nc localhost 7000` |
| `daytime-server` | 1313 | `daytime-client [värd] [port]` eller `nc localhost 1313` |
| `chatserver-single-threaded` | 4711 | `nc localhost 4711`, gärna två stycken |
| `chatserver-multi-threaded` | 4711 | `nc localhost 4711` i flera terminaler, eller simulatorn |

Simulatorn, 200 klienter som skickar 20 rader var med slumpmässiga mellanrum:

```sh
client-simulator/chat-sim.py --clients 200 --messages 20
```

### Kostnaden med tråd per klient

Trådar medan simulatorn kör. N klienter ger N + 2 trådar (accept, broadcast och N läsare):

```sh
ps -T -p $(pgrep -f build/chatserver-multi-threaded) | wc -l
```

Head-of-line blocking: `asio::write` blockerar, så en klient som inte läser stoppar broadcast för
alla. Anslut en `nc` och pausa den med Ctrl-Z, anslut en till, och låt en tredje klient skicka
tills kärnans socketbuffertar är fulla. På loopback krävs några MB innan det händer:

```sh
python3 -c 'import socket; s=socket.create_connection(("localhost",4711)); [s.sendall(b"x"*200+b"\n") for _ in range(40000)]'
```

Den andra `nc`:n slutar få meddelanden. Ta tillbaka den pausade med `fg`, så lossnar det.
Asynkron skrivning med en kö per session löser detta i del 2.

## Fördjupning

Detaljer som artikeln bara snuddar vid:

- **Samtidig läsning och skrivning på samma socket.** Asios dokumentation för `basic_stream_socket`
  säger *"Shared objects: Unsafe"*, men också att synkrona `send`, `receive`, `connect` och
  `shutdown` är trådsäkra i förhållande till varandra. I chatt 1b läser läs-tråden och skriver
  broadcast-tråden på samma socket, vilket alltså är tillåtet. `close` är inte trådsäkert, så
  socketen stängs först i `Session`-destruktorn när sista `shared_ptr` släpper den.
- **`read_until` läser förbi avgränsaren.** Returvärdet `n` är radens längd, men bufferten kan
  innehålla början på nästa rad. Ta därför bort exakt `n` byte, inte hela strängen.
- **Läs-trådarna är `std::thread` med `detach()`,** inte `std::jthread`. En `jthread` måste sparas
  och joinas någonstans, och en lista med döda trådar läcker tills någon joinar dem. Broadcast-tråden
  är en `jthread`, där `stop_token` gör nytta i köns `pop()`.
- **`BOOST_ASIO_DISABLE_ERROR_LOCATION`** sätts i `common.cmake`. Utan den innehåller
  `system_error::what()` sökväg, radnummer och funktionsnamn från Asios interna headers.
- **Servrarna lyssnar på IPv4.** `localhost` resolvas ofta till både `::1` och `127.0.0.1`, och
  `asio::connect` provar endpoints i tur och ordning tills en lyckas. Det är därför klienten
  fungerar utan att man märker något.

## Relaterade artiklar

- [Kom igång med Boost](https://www.ribomation.se/blog/2026/kom-igang-med-boost/)
- [Väderdata med Boost](https://www.ribomation.se/blog/2026/vaderdata-med-boost/)
- [Asynkrona operationer med coroutines](https://www.ribomation.se/blog/2026/asynkrona-operationer-med-coroutines/)
