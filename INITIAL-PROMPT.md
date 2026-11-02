# Demo program för artikeln &amp; Bygg en chattserver med Boost.Asio, del 1: Grunderna och synkron I/O

## Länkar

- /home/jensr/GitHubProjects/app-www-11ty-v3/brainstorming/boost-asio-artikelserie.md
- /home/jensr/GitHubProjects/app-www-11ty-v3/brainstorming/del-1-grunderna.md


## Externa bibliotek
Alla externa (3:de parts) bibliotek finns installerade i mappen 

- /home/jensr/Libs/

Om något saknas bör detta först installeras enligt instruktioner i 

- /home/jensr/Libs/README.md


## CMake

Länka externa bibliotek, såsom Boost, via find* modules i 

- /home/jensr/Libs/CMake-modules

En parent cmake file i project-roten, som länkar in Boost Asio plkus ev. andra.
En cmake file per demo program i undermappar.
Det ska gå att bygg respektive demoprogram via cmake i respektive mapp, men
också bygga samtliga via cmake i projektroten.


## Mappstruktur

asio-chatt-del-1/
    - CMakeLists.txt
    - echoserver/
        - CMakeLists.txt
        - src/*
    - daytime/
        - CMakeLists.txt
        - client/*
        - server/*
    - chatserver-single-threaded
        - CMakeLists.txt
        - src/*
    - chatserver-multi-threaded
        - CMakeLists.txt
        - src/*


## Arbetsgång
Bygg ett demoprogram i taget (i ordningsföljd) och vänta på att Jens gör review och godkänner.
För varje program i samband med att vänta på review, sammanfatta vad som har byggts och visa
kommandon för att bygga och köra programmet (copy & paste), samt förväntat utdata.


## C++ stil
Använd filändelser `*.cxx` och `*.hxx` för implementationsfiler och headerfiler respektive.
Använd `auto` så mycket som möjligt:

- Local variables
- Trailing return types, IFF the length of the return type is greater than 4 chars

Välj Modern C++, när det gäller val av klasser och funktioner i standardbiblioteket. Använd
ranges and views i stället för classic STL, när det går. Fråga Jens om du behöver råd.


## Versionhantering

I samband med start av detta, initiera git i mappen och sen skapa ett publikt GitHub repo med
samma namn. Efter att varje program blivit godkänt av Jens, committa och pusha till github.
När alla program är klara, skriv en README.md, som beskriver innehållet och målsättningen med
projektet, samt länka till artikeln som demo programmen tillhör.


## Frågor
Om något är oklart, fråga Jens om detta, med en fråga i taget.


