# Összejátszás, közös mezőkön                            

# Bevezetés

Az összejátszásban résztvevő mezők:

```
         ╔══╦══╦══╦══╗
         ║  ║  ║  ║  ║
╔══╦══╦══╬══╬══╬══╬══╬══╦══╦══╗
║L1║L4║X1║  ║  ║  ║  ║X4║R1║R4║
╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
║L2║L5║X2║  ║  ║  ║  ║X5║R2║R5║
╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
║L3║L6║X3║  ║  ║  ║  ║X6║R3║R6║
╚══╩══╩══╬══╬══╬══╬══╬══╩══╩══╝
         ║  ║  ║  ║  ║
         ╚══╩══╩══╩══╝

Li,Ri: ezeken a mezőkön van összejátszás
X: ezeken a mezőkön nincsen összejátszás, de a szívességeket,
	amit a védekező tesz, azt számon tartjuk.

```

A taktika célja, hogy a közös mezőkön történő támadásra előre lerögzítjük,
mit válaszol a védekező, vagy milyen mezők közül választhat.
Összejátszás esetén tehát a 3 lehetséges védekező lépés:
* közös mezőre fixen védekezünk
* szomszéd válaszolhat pár mező között
* mi válaszolhatunk pár mező közül.

2 féleképpen kezdheti a támadó a közös mezőkön:
(az x tengelyre természetesen adódik, hogy szimmetrikus a taktika,
az y tengelyre pedig azért esnek azonos kategóriába a szimmetrikus mezők, mert közösen akarnak válaszolni, tehát egyben kell őket kezelni)
```
         ╔══╦══╦══╦══╗
         ║  ║  ║  ║  ║
╔══╦══╦══╬══╬══╬══╬══╬══╦══╦══╗
║A ║A ║  ║  ║  ║  ║  ║  ║A ║A ║
╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
║B ║B ║  ║  ║  ║  ║  ║  ║B ║B ║
╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
║A ║A ║  ║  ║  ║  ║  ║  ║A ║A ║
╚══╩══╩══╬══╬══╬══╬══╬══╩══╩══╝
         ║  ║  ║  ║  ║
         ╚══╩══╩══╩══╝
```

Megjegyzés: általában az olyan esetek nincsenek lekezelve, ahol a meglévő közös mezőkbe védekező tesz.
A kódban ilyenkor megkap minden közös mezőt az adott oldal. [Ezeket majd ki kell javítani...]

# A eset
* Válaszlépés a támadó oszlopában a középső mező
* A foglalt 2 mezővel egy sorban levő 2 mezőt megkapja a rosszabbul álló játékos, de azt a vonalat neki kell lefogni
* A maradék 2 mezőn folytatódik az összejátszás

Példa:
```
         ╔══╦══╦══╦══╗
         ║  ║  ║  ║  ║
╔══╦══╦══╬══╬══╬══╬══╬══╦══╦══╗
║O1║. ║  ║  ║  ║  ║  ║  ║O1║O ║
╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
║X2║. ║  ║  ║  ║  ║  ║  ║X2║O ║
╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
║  ║  ║  ║  ║  ║  ║  ║  ║  ║  ║
╚══╩══╩══╬══╬══╬══╬══╬══╩══╩══╝
         ║  ║  ║  ║  ║
         ╚══╩══╩══╩══╝
Ugyanaz a támadás, két oldalról szemlélve:
- jobb oldalon egy gyenge kezdésre válaszolhattunk a legerősebb védekezéssel, ezért 2 mezőt atadunk a túloldalnak
- bal oldalon szinte visszakaptuk a kezdőállást, nehéz helyzetbe kerültünk, és még össze is kell játszani.
Ezért a számára a két leghasznosabb mezőt megkapja.
```
A bal oldalon valójában nem kell lefogni a legszélső 3-as vonalat, ezért valójában így néz ki számára a tábla:
```
         ╔══╦══╦══╦══╗
         ║  ║  ║  ║  ║
╔══╦══╦══╬══╬══╬══╬══╬══╦══╦══╗
║  ║  ║  ║  ║  ║  ║  ║  ║O1║O ║
╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
║  ║  ║  ║  ║  ║  ║  ║  ║X2║X ║
╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
║  ║  ║  ║  ║  ║  ║  ║  ║  ║  ║
╚══╩══╩══╬══╬══╬══╬══╬══╩══╩══╝
         ║  ║  ║  ║  ║
         ╚══╩══╩══╩══╝
```

Megjegyzés:
könnyű lenne a dolgunk, ha a bal oldali játékos a fenti ábrán oda tudná adni a az oszlopába eső harmadik mezőt a szomszédjának, cserébe a másik 3 mezőért. De ez sajnos nem működik, mert az alábbi állás
```
         ╔══╦══╦══╦══╗
         ║  ║  ║  ║  ║
╔══╦══╦══╬══╬══╬══╬══╬══╦══╦══╗
║O1║. ║  ║  ║  ║  ║  ║  ║. ║O3║
╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
║X2║. ║  ║  ║  ║  ║  ║  ║. ║X4║
╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
║O ║. ║  ║  ║  ║  ║  ║  ║. ║O ║
╚══╩══╩══╬══╬══╬══╬══╬══╩══╩══╝
         ║  ║  ║  ║  ║
         ╚══╩══╩══╩══╝
. : a szomszédtól megkapott mezők
O : az átadott 1-1 mező
```
támadó számára győztes állás.

## A.1
* A kevésbé szorongattatott játékos térfelére érkezik a támadás.
* A kedvezményezett játékos sajnos nem tehet a megmaradt közös mezőre, különben kikapna, de a 7-es vonalat le tudja fogni a szomszédnak [ha nem tud oda tenni, a közös mezőre tesz]
```
Az állás a gyengébb és erősebb oldal szemszögéből:
           ╔══╦══╦══╦══╗
           ║  ║  ║  ║  ║
  ╔══╦══╦══╬══╬══╬══╬══╬══╦══╦══╗
  ║O1║. ║  ║  ║  ║  ║  ║  ║O1║O ║
  ╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
  ║X2║. ║  ║  ║  ║  ║  ║  ║X2║X ║
  ╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
X4║O3║  ║  ║  ║  ║  ║  ║X4║O3║  ║
  ╚══╩══╩══╬══╬══╬══╬══╬══╩══╩══╝
           ║  ║  ║  ║  ║
           ╚══╩══╩══╩══╝
```
* plusz a jobb oldali játékos átadja a megmaradt közös mezőt a szomszédnak

Tehát így néz ki az új állás a 2-2 oldalról:
```
           ╔══╦══╦══╦══╗
           ║  ║  ║  ║  ║
  ╔══╦══╦══╬══╬══╬══╬══╬══╦══╦══╗
  ║O1║. ║  ║  ║  ║  ║  ║  ║O1║O ║
  ╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
  ║X2║. ║  ║  ║  ║  ║  ║  ║X2║X ║
  ╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
X4║O3║. ║  ║  ║  ║  ║  ║X4║O3║O ║
  ╚══╩══╩══╬══╬══╬══╬══╬══╩══╩══╝
           ║  ║  ║  ║  ║
           ╚══╩══╩══╩══╝
```

## A.2
Nagyobb szívás, ha a másik mezőre érkezik támadás, ekkor a bal oldali játékos válaszolhat, de muszáj szívességet tennie a baloldali játékosnak, különben az kikap.
Ezt nem sikerült korrektre kihozni, tehát a csalás benne az, hogy az egyik esetben az összejátszás után mindkét játékos megkapja a megmaradt utolsó közös mezőt.

Az alapvető elgondolás az, hogy a jobb oldali játékos lefog 1-2 bal oldali 7-es vonalat pluszba, és akkor szabadon válaszolhat.
Pontosabban:

```
           ╔══╦══╦══╦══╗
           ║  ║  ║  ║  ║
  ╔══╦══╦══╬══╬══╬══╬══╬══╦══╦══╗
  ║O1║P1║P2║  ║  ║  ║  ║  ║O1║O ║L1
  ╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
  ║X2║. ║  ║  ║  ║  ║  ║  ║X2║X ║
  ╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
  ║P4║O3║P3║  ║  ║  ║  ║  ║K ║O3║L2
  ╚══╩══╩══╬══╬══╬══╬══╬══╩══╩══╝
           ║  ║  ║  ║  ║
           ╚══╩══╩══╩══╝
Pi : bal oldali játékosnak segítség
```
A jobb oldali játékos a következőképpen lép:
* Ha van védekező szimbólum P1-en, vagy (P2&P3)-on, akkor szabadon védekezhet
* Ha P2 vagy P3-on van védekező szimbóluma, szintén szabadon védekezhet, de át kell adnia P4-et
[ITT csaltam, nem adja át P4-et, különben veszít]
* Ha egyik se teljesül, akkor választhat P1,P2,P3,P4 valamelyike közül.
[ITT is csaltam, át kellene adnia P2 és P3 esetén P4-et a szomszédnak, de ez nincs benne]

A bal oldali játékos szempontjából így néz ki a dolog:
Támadó választ az alábbiak közül:
* L1 & L2 védekező szimbólum, K-t át kell adnia [ez nincs még benne]
* L1 és az 5-ös vonal le van fogva, de K-t át kell adnia [ez sincs még benne]
* L1 vagy L2 védekező szimbólum, de K-t megkapja
* K-t megkapja [ez BUG, valamit kell cserébe kapnia]

Tehát a teendő itt az utolsó mezőn az összetátszás kidolgozása...

# B
Védekező válaszai:
```
         ╔══╦══╦══╦══╗
         ║  ║  ║  ║  ║
╔══╦══╦══╬══╬══╬══╬══╬══╦══╦══╗
║  ║. ║X2║  ║  ║  ║  ║  ║. ║  ║
╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
║  ║O1║  ║  ║  ║  ║  ║X2║O1║  ║
╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
║  ║. ║  ║  ║  ║  ║  ║  ║. ║  ║
╚══╩══╩══╬══╬══╬══╬══╬══╩══╩══╝
         ║  ║  ║  ║  ║
         ╚══╩══╩══╩══╝
. : már nem közös mező
```
* az adott oszlop mezői elvisztik közös jellegüket, hiszen 2-es vonal lesz

## B.1
A támadó következő lépése is középre jön, a fenti lépések ismétlődnek meg.
==> Nincs több közös mező!
```
           ╔══╦══╦══╦══╗
           ║  ║  ║  ║  ║
  ╔══╦══╦══╬══╬══╬══╬══╬══╦══╦══╗
  ║. ║. ║X2║  ║  ║  ║  ║  ║. ║. ║X4
  ╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
X4║O3║O1║  ║  ║  ║  ║  ║X2║O1║O3║
  ╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
  ║. ║. ║  ║  ║  ║  ║  ║  ║. ║. ║
  ╚══╩══╩══╬══╬══╬══╬══╬══╩══╩══╝
           ║  ║  ║  ║  ║
           ╚══╩══╩══╩══╝
. : már nem közös mező
```

## B.2.1
```
           ╔══╦══╦══╦══╗
           ║  ║  ║  ║  ║
  ╔══╦══╦══╬══╬══╬══╬══╬══╦══╦══╗
  ║O3║. ║X2║  ║  ║  ║  ║  ║O3║. ║X2
  ╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
  ║X4║O1║  ║  ║  ║  ║  ║  ║X4║O1║
  ╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
  ║O ║. ║  ║  ║  ║  ║  ║  ║. ║. ║
  ╚══╩══╩══╬══╬══╬══╬══╬══╩══╩══╝
           ║  ║  ║  ║  ║
           ╚══╩══╩══╩══╝
. : már nem közös mező
O : átadott mező
```

## B.2.2
```
           ╔══╦══╦══╦══╗
           ║  ║  ║  ║  ║
  ╔══╦══╦══╬══╬══╬══╬══╬══╦══╦══╗
  ║. ║O3║X4║  ║  ║  ║  ║  ║. ║O3║X4
  ╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
X2║O1║. ║  ║  ║  ║  ║  ║X2║O1║. ║
  ╠══╬══╬══╬══╬══╬══╬══╬══╬══╬══╣
  ║. ║. ║  ║  ║  ║  ║  ║  ║. ║. ║
  ╚══╩══╩══╬══╬══╬══╬══╬══╩══╩══╝
           ║  ║  ║  ║  ║
           ╚══╩══╩══╩══╝
. : már nem közös mező
O : átadott mező
```
