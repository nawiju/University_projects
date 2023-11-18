## Heroes 3
Przykładowa rozgrywka: https://cdn2.unrealengine.com/screens-02-3840x2160-93314bf113a1.jpg?h=720&quality=medium&resize=1&w=1280

Co można z niej zaobserwować?
* dwóch graczy
* plansza złożona z sześciokątów (wysokość i szerokość mogą być parametrami)
* na planszy są przeszkody - kamienie (na razie je zignorujmy)
* na planszy są jednostki:
    * niektóre są latające (mogą przelatywać nad przeszkodami i innymi jednostkami)
    * niektóre są strzelające
    * niektóre zajmują dwa pola
    * mają określony zasięg ruchu
* jednostki mają liczebność (i zapewne inne cechy)
* na planszy są dodatkowe obiekty (na razie je zignorujmy):
    * namiot medyczny
    * balista
    * wózek z amunicją
* algorytm ruchu jednostek jest następujący:
    * wszystkie jednostki trafiają do jednej listy, posortowane po zasięgu ruchu (inicjatywie)
    * wybieramy (i usuwamy z listy) jednostkę o największym zasięgu ruchu, po czym dajemy graczowi listę możliwych akcji, które może wykonać:
      * możliwość ruszenia się na dane pole (zwykłe jednostki nie mogą przechodzić na ani przez pola z przeszkodami ani innymi jednostkami, ale latające mogą)
      * możliwość ruszenia się i ataku jednostki przeciwnika stojącej obok pola na które chcemy się ruszyć
      * możliwość czekania (jeśli to możliwe)
      * możliwość bronienia się
      * możliwości strzału - dla jednostek strzelających (możemy strzelić do dowolnej jednostki przeciwnika)
      * możliwość zaatakowania dowolnej jednostki przeciwnika stojącej na polu obok nas
    * gracz zwraca nam swoją akcję (przesunięcie się na jedno z pól, czekanie, obrona, a w przyszłości to może być przesunięcie i atak, atak w miejscu i strzał)
    * jeśli jednostka czeka, to dodajemy ją na koniec listy (każda jednostka może czekać tylko raz na rundę)
    * jeśli jednostka czeka, to dostaje bonus do obrony (działający do końca rundy) i nie może się już ruszać w tej rundzie; na razie wystarczy wywołać jakąś funkcję "zaślepkę", statystyki jednostek wprowadzimy później
    * jeśli jednostka się rusza, to zmieniamy jej pozycję
    * przechodzimy do następnej jednostki
    * jak lista będzie pusta, to zaczynamy nową rundę
    * WAŻNE: w przyszłości mogą pojawić się modyfikatory zasięgu jednostek, które spowodują zmiany w kolejności ruchu

## Zadanie domowe na 15 maja 2023:
Zaimplementuj hierarchię klas potrzebną do gry. W szczególności napisz:
* algorytm znajdowania możliwych pól ruchu, z uwzględnieniem tego, że niektóre jednostki mogą latać
* algorytm ruchu jednostek (opisany wyżej); gracze mogą wykonywać wszystkie opisane tam akcje, ale na razie możecie nie robić implementacji akcji ataku i strzału
* wypisywanie stanu planszy w formie tekstowej, tak żeby dawało się debugować program - Wy będziecie je debugować, więc napiszcie tak, żeby Wam było wygodnie

Możesz rozwijać dostarczone klasy lub rozwijać własny projekt od zera.


## Zadanie domowe na 12 czerwca 2023:

UWAGA WSTĘPNA - JEDNOSTKA A ODDZIAŁ:
Jednostka to formalnie jakiś rodzaj kreatury, np Pikinier, Anioł, Zielony Smok itp razem ze wszystkimi statystykami i atrybutami (atak, obrona, czy lata, czy strzela itd).
Oddział to grupa jednostek tego samego rodzaju, np. 5 Pikinierów, który może zajmować jakieś pole na planszy. Oddział ma dodatkowo informację o tym, czy jedna jednostka w oddziale ma niepełne życie (np. 5 Pikinierów po 10 życia i 1 Pikinier po 5 życia).
W opisie zadania używałem sformułowania `jednostka`, ale najczęściej mam na myśli `oddział`. 

Obowiązkowo powinno się pojawić (łącznie za 1,5 punkta):
* statystyki jednostek:
    * atak
    * obrona
    * liczba strzał (jeśli jednostka strzelająca) - czyli ile razy w danej grze jednostka może strzelić
    * jednostki życia
    * obrażenia (liczba lub zakres), np 30 lub 20-40
    * szybkość (inicjatywa)
    * typ naziemny / latający
    * przykłady są tutaj: http://h3.heroes.net.pl/jednostki/zamek
* na dany oddział gracza składa się N jednostek, z czego jedna może mieć niepełne życie (np. 5 jednostek po 10 życia, 1 jednostka po 5 życia); przy otrzymywaniu obrażeń najpierw tracą życie jednostki niepełne, a potem pełne
* algorytm liczenia obrażeń: https://mightandmagic.fandom.com/wiki/Damage_(Heroes)
* możliwość zaatakowania jednostki przeciwnika wręcz:
    * można zaatakować jednostkę przeciwnika stojącą na polu obok nas
    * można przejść na inne pole (zgodnie z algorytmem ruchu) i zaaatakować jednostkę przeciwnika stojącą obok tego pola (uwaga, tych jednostek może być kilka do wyboru)
* 4-5 przykładowych jednostek ze statystykami z gry
* stan początkowy gry inicjowany statycznie, tzn bez wczytywania z pliku/standardowego wejścia
* gracze wykonują albo losowe ruchy, albo ruchy zdefiniowane w kodzie (np. najpierw jednostka A atakuje jednostkę B, potem jednostka C atakuje jednostkę D itd), albo wczytują ruchy ze standardowego wejścia (wystarczy zaimplementować jedną z tych opcji)
* gra kończy się w momencie, w którym na planszy zostaną jednostki tylko jednego gracza
Żeby dostać te 1,5 punkta, projekt powinien być w stanie skończonym tzn bez ficzerów zaimplementowanych w połowie, klasy powinny być uporządkowane, podzielone na pakiety. Styl kodu powinien być akceptowalny. Lepiej jest zrobić mniej ficzerów, ale porządnie.

Rozszerzenia:

Jeśli pojawia się nowa cecha jednostki (strzał, ataki obszarowe), to powinna pojawić się też jednostka, która posiada tą cechę

* kontratak (0,5p) - każda jednostka wykonuje kontratak na pierwszy atak wręcz w danej rundzie, tzn po tym jak otrzyma obrażenia, automatycznize sama zadaje obrażenia jednostce, która ją zaatakowała
* możliwość strzału (1p):
    * jednostki strzelające mogą strzelać do dowolnej jednostki przeciwnika
    * strzał do jednostek, które znajdują się dalej niż 10 pól od strzelca powoduje zmniejszenie obrażeń o połowę
    * jednostki nie kontratakują strzałów
    * jeśli obok jednostki strzelającej stoi jednostka przeciwnika, to strzelec nie może strzelać - może jedynie atakować wręcz, ale powoduje to zmniejszenie obrażeń o połowę
* jednostki atakujące obszarowo (0,5p):
    * smoki atakują daną jednostkę i jednostkę stojącą za nią w tej samej linii
    * niektóre jednostki strzelające (Lisz - http://h3.heroes.net.pl/je]dnostki/nekropolia) atakują daną jednostkę i 6 pól wokół niej
* morale i szczęście (1p): http://h3.heroes.net.pl/umiejetnosci/umiejetnosci-podstawowe (opis na dole strony)
    * wartość morale to suma:
        * cechy gracza (zakładamy, że jest to jakaś konkretna wartość w momencie tworzenia gracza)
        * wpływu na morale jednostek gracza - jeśli wszystkie jednostki są z jednej frakcji, to morale = 1, jeśli dokładamy jednostki z innej frakcji, to morale spada o 1 za każdą nową frakcję; innymi słowy, morale = 2 - liczba frakcji w armii gracza
    * szczęście to cecha gracza (zakładamy, że jest to jakaś konkretna wartość w momencie tworzenia gracza)
* magia (2p):
    * http://h3.heroes.net.pl/magia
    * gracze zaczynają grę z pewną liczbą punktów magii i określoną mocą
    * każdy czar ma swój koszt w punktach magii
    * każdy gracz może rzucić jeden czar na rundę (w momencie, gdy ma wykonać akcję swoją jednostką, ale przed wykoneniem tej akcji; po rzuceniu czaru gracz wykonuje akcję jednostką)
    * moc bohatera wpływa na siłę rzucanych czarów
    * zakładamy, że bohaterowie rzucają zaklęcia na poziomie zaawansowanym (na podstawie Magicznej strzały: powinna ona zadawać `10 * moc + 20` obrażeń)
    * Typy czarów:
        * niektóre czary (np. Magiczna strzała) zadają obrażenia,
        * inne (np. Żądza krwi, Błogosławieństwo, Przyspieszenie) czasowo modyfikują statystyki oddziału,
        * jeszcze inne mogą usuwać przeszkody, atakować losowe pola na planszy, ożywiać umarłe jednostki, przywoływać nowe jednostki, przejmować kontrolę nad oddziałem przeciwnika itp.
    * Żeby dostać 2 punkty, trzeba dodać co najmniej 4 różne czary:
        * max 2 czary zmieniające czasowo statystyki jednostek,
        * max 1 czar zadający obrażenia,
        * min 1 czary z tej listy: Hipnoza, Ruchome piaski, Wskrzeszenie, Klonowanie, Ściana ognia, Berserk

    
## Pomysły na rozwój projektu:
* zaimplementować GUI (to w Javie jest problematyczne, bo mamy aż 3 różne GUI: AWT, Swing i JavaFX, z czego żadne nie jest szczególnie wygodne ani popularne), a aktualnie większość interfejsów jest pisanych w Electronie z użyciem JavaScriptu i HTMLa
* zaimplementować interfejs tekstowy w konsoli (nie wszyscy to lubią)
* zaimplementować grę po sieci:
  * wersja 1: jeden komputer jest serwerem i pierwszym graczem, a drugi klientem - drugim graczem, komunikacja odbywa się po UDP (bonus za oldschool vibes)
  * wersja 2: serwer jest niezależny od graczy, do serwera łączą się dwa klienty, komunikacja odbywa się po UDP - jeden serwer mógłby obsługiwać wiele rozgrywek jednocześnie, ale to już sporo pracy
  * wersja 3: serwer jest niezależny od graczy, komunikacja odbywa się po RESTowym API HTTP; klientem byłby wtedy dowolny klient HTTP, np Postman, bo tutaj nie ma szans, żebyśmy się wyrobili z jakimkolwiek GUI
