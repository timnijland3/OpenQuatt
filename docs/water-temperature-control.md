# Water Temperature Control

Deze pagina is bedoeld voor gebruikers die de stooklijnregeling van OpenQuatt beter willen begrijpen, zonder meteen in technische details te verdwijnen.

> Zoek je vooral de korte uitleg? Begin dan bij [Verwarmen en comfort](dagelijks/verwarmen.md).

## Wat is Water Temperature Control?

`Water Temperature Control` is de strategie die vooral denkt in gewenste aanvoertemperatuur.

De gedachte is:

- hoe koud is het buiten;
- welke aanvoertemperatuur hoort daar volgens de stooklijn bij;
- hoe ver zit de echte aanvoer daarvan af;
- hoeveel compressorvraag is nodig om dat doel te volgen.

Dit voelt vaak vertrouwder voor gebruikers die gewend zijn aan een klassieke stooklijn.

## Wanneer kies je dit?

Deze strategie past meestal goed als je:

- graag in een stooklijn denkt;
- vooral naar aanvoertemperatuur kijkt;
- een voorspelbare weersafhankelijke regeling wilt;
- liever minder directe kamercorrectie gebruikt dan bij `Power House`.

Wil je juist sterker op comfort en kamertemperatuur sturen, dan past [Power House](power-house.md) vaak beter.

## Hoe werkt het in gewone taal?

De regeling loopt in de praktijk in vier stappen.

### 1. De buitentemperatuur bepaalt het doel

OpenQuatt gebruikt de gekozen buitentemperatuur om een aanvoertemperatuurdoel te bepalen.

Belangrijk:

- als de buitentemperatuurbron niet klopt, klopt de hele stooklijn ook niet;
- een te lage buitenwaarde geeft vaak een te hoge gevraagde aanvoer;
- een te hoge buitenwaarde geeft vaak een te lage gevraagde aanvoer.

### 2. De stooklijn maakt een supply target

De stooklijn is opgebouwd uit een paar vaste buitentemperatuurpunten. Tussen die punten rekent OpenQuatt vloeiend door.

Belangrijke instellingen:

- `Curve Tsupply @ -20°C`
- `Curve Tsupply @ -10°C`
- `Curve Tsupply @ 0°C`
- `Curve Tsupply @ 5°C`
- `Curve Tsupply @ 10°C`
- `Curve Tsupply @ 15°C`
- `Curve Fallback Tsupply (No Outside Temp)`

Hiermee bepaal je dus vooral: welke aanvoer hoort bij welk weer?

### 3. De PID houdt de aanvoer op koers

Daarna vergelijkt OpenQuatt het doel met de gemeten aanvoer.

Belangrijke instellingen:

- `Heating Curve PID Kp`
- `Heating Curve PID Ki`
- `Heating Curve PID Kd`

In gewone taal:

- `Kp` bepaalt de directe reactie;
- `Ki` werkt blijvende afwijking weg;
- `Kd` dempt snelle schommelingen.

Voor de meeste installaties is `Kp` en `Ki` belangrijker dan `Kd`.

### 4. Begrenzing en laaglastgedrag houden het rustig

OpenQuatt laat de regeling niet zomaar hard naar nul vallen als de vraag klein wordt.

Praktisch merk je dan:

- minder pendelen rond lage vraag;
- rustigere overgang naar uit;
- minder vaak snel opnieuw starten.

Ook hier blijft `Maximum water temperature` belangrijk als rem en beveiliging.

## Wat doet de kamer nog in deze modus?

De kamer blijft wel meetellen, maar niet als hoofdregeling.

Praktisch:

- als de ruimte al duidelijk te warm wordt, kan OpenQuatt het stooklijndoel wat afremmen;
- als de ruimte iets te koud is, blijft de stooklijn zelf het hoofdmodel;
- deze strategie is dus minder direct kamergericht dan `Power House`.

Dat maakt `Water Temperature Control` vaak rustiger en voorspelbaarder, maar soms ook minder "comfortgestuurd".

## Warmtetoestemming: meestal de thermostaat

Bij `Water Temperature Control` bepaalt de buitentemperatuur via de curve het gewenste aanvoerdoel; de PID regelt daarna de werkelijke aanvoer naar dat doel. De kamer is een aanvullende correctie, niet de primaire vraagregelaar.

Daarom is bij een normale installatie met kamerthermostaat een externe warmtetoestemming juist logisch: de thermostaat/zone-regeling bepaalt *of* verwarming nodig is, OpenQuatt bepaalt met de stooklijn *hoe warm* het water moet zijn.

Tijdens Quick Start zet een strategieswitch naar `Water Temperature Control` de warmtetoestemming automatisch op de eerder gekozen, gekoppelde thermostaatbron. Dat is op de Q-edition normaal `Heating Enable Source = OT thermostat`; op andere hardware kan dit `CIC` of `HA input` zijn. Een uitgeschakelde of niet-geconfigureerde bron wordt niet automatisch als harde gate gekozen. `Niet gebruiken` blijft mogelijk voor bewust volledig weersafhankelijk bedrijf met permanent open afgiftesysteem.

`Heating Enable Source = Niet gebruiken` betekent: geen externe warmtetoestemming; de strategie mag zelf warmtevraag opbouwen zonder harde gate. Met `Niet gebruiken` kan de stooklijn dus ook verwarmen terwijl de kamer al boven setpoint is.

Buiten Quick Start toont `Instellingen → Verwarmen` bij een afwijkende keuze alleen een advies (`Aanbevolen instelling` / `Controleer configuratie`) en wordt de instelling niet stil overschreven. Voor de volledige matrix zie [Instellingen en meetwaarden](instellingen-en-meetwaarden.md#5-bronselectie).

## Stooklijn instellen en tijdelijk verschuiven

De web-app biedt onder `Instellingen → Verwarmen` een Simple-editor met helling
(K per 10 °C kouder weer) en niveau (aanvoerdoel bij 0 °C buiten). De preview
toont vooraf de zes punten die `Toepassen` naar de bestaande stooklijn schrijft.
Onder Advanced kun je alle zes punten rechtstreeks blijven wijzigen. Simple
opnieuw toepassen vervangt een handmatig gevormde Advanced-curve.

Met **Stooklijn-offset** (`Heating Curve Modifier Source`) kun je de lokale stooklijn tijdelijk
verschuiven via `HA input`, `API input` of `MQTT`. De aanvoercorrectie wordt
begrensd op −5…+5 °C en vóór de bestaande kamertrim en waterlimiet verwerkt.
Bij een ongeldige of verouderde bron geldt 0 °C correctie; de lokale stooklijn
blijft actief. Een temperatuurverschil van 1 °C is numeriek gelijk aan 1 K;
de firmware-entiteiten kunnen daarom nog `K` als eenheid tonen.
Gebruik voor HA `sensor.openquatt_ext_heating_curve_modifier` en
`binary_sensor.openquatt_ext_heating_curve_modifier_valid` uit het
`dynamic-sources`-pakket. De HA-heartbeat moet vers zijn; er is voor deze
nieuwe bron geen tijdloze legacy-fallback. Een geldig absoluut extern
aanvoertarget vervangt nog steeds het hele lokale resultaat en krijgt de
offset niet nogmaals toegepast.

De sensoren `Heating Curve Base Target`, `Heating Curve Modifier`,
`Heating Curve Room Trim` en `Heating Supply Target (Effective)` tonen de
opbouw en het uiteindelijk gekozen doel afzonderlijk.

## Extern aanvoertarget (optioneel)

Standaard bepaalt de stooklijn het aanvoerdoel uit de buitentemperatuur, met een kleine kamertrim erbovenop. Laat je dat doel liever door een externe regelaar bepalen — bijvoorbeeld een OpenTherm-thermostaat die zelf al kamerafwijking in `TSet` verwerkt, een buffervatregeling in Home Assistant of een MPC-optimalisatie — dan kun je dat target rechtstreeks doorgeven.

Zet daarvoor `Heating Supply Target Source` op `OT thermostat`, `HA input`, `API input` of `MQTT` en lever een waarde in graden Celsius. Staat de bron op `Heating curve`, dan verandert er niets aan het gedrag dat je nu kent.

Voor `HA input` luistert OpenQuatt naar twee vaste entiteiten in Home Assistant:

- `sensor.openquatt_ext_heating_supply_target` — het gewenste aanvoerdoel in graden Celsius;
- `binary_sensor.openquatt_ext_heating_supply_target_valid` — moet `on` staan, anders negeert OpenQuatt de waarde en valt hij terug op de stooklijn.

Die tweede entiteit is jouw eigen geldigheidsschakelaar: zet hem `off` zodra je berekening verouderd of onbetrouwbaar is. Uitzetten werkt direct: een eerder vastgehouden waarde wordt meteen ingetrokken en de stooklijn neemt het weer over, zonder overbruggingsvenster. Alleen waarden binnen 20…70 °C tellen mee.

Publiceer de waarde als hij verandert; periodiek opnieuw publiceren van een ongewijzigde waarde is niet nodig. Het `dynamic-sources`-pakket in Home Assistant levert namelijk een centrale heartbeat (`sensor.openquatt_ha_ingress_heartbeat`), waarvan de state zelf ongeveer elke minuut verandert. Zolang die heartbeat binnenkomt, blijft ook een constante waarde (bijvoorbeeld 40,0 °C) gewoon bruikbaar. Blijft de heartbeat langer dan 15 minuten uit — bijvoorbeeld bij een verbroken Home Assistant-verbinding — dan wordt de waarde ongeldig. Bij een korte Home Assistant-herlaadactie kan OpenQuatt de laatst geldige waarde daarna nog maximaal 5 minuten overbruggen; zet de geldigheidsentiteit expliciet `off` om die overbrugging direct te beëindigen. Zo valt een bevroren waarde uiterlijk na 20 minuten terug op de stooklijn. Met het actuele `dynamic-sources`-pakket is de ingang na een herstart ongeldig tot de eerste heartbeat én een geldige waarde binnen zijn. Zonder heartbeat blijft voor dit aanvoertarget bewust de oude publicatiegebonden freshness-regel gelden, zodat een bevroren target ook na een firmware-update blijft verlopen. Voor `API input` en `MQTT` stuur je de waarde naar een lokaal endpoint of topic; zie [API inputbronnen](api-input.md) en [MQTT inputbronnen](mqtt.md). Die wegen hebben geen aparte geldigheidsentiteit en vervallen zonder extra overbrugging na 15 minuten. Voor `OT thermostat` gebruikt OpenQuatt de al ontvangen `OT - Control Setpoint` (`TSet`), met een eigen versheidsbewaking: zonder recent `TSet`-bericht valt de regeling terug op de stooklijn. Ook hier telt alleen een `TSet` binnen 20…70 °C mee: `TSet=0` (thermostaat zonder warmtevraag) of een andere waarde buiten bereik valt terug op de stooklijn in plaats van als extern minimumtarget te gelden.

Wat die externe waarde wel en niet doet:

- Hij vervangt alleen het berekende stooklijntarget, inclusief kamertrim. De trim wordt bij een externe target niet nogmaals toegepast, anders zou de correctie dubbel tellen.
- `Maximum water temperature`, de water-temperatuurlimieten en trips, de PID, de demandlogica en de compressor- en Duo-dispatch blijven van OpenQuatt zelf.
- Valt de bron weg, wordt hij te oud of stuurt hij een onbruikbare waarde, dan gaat de regeling terug naar de eigen stooklijn. Niet naar nul of uit.

Dit staat los van `Heating Enable`: die bepaalt *of* er verwarmd mag worden, het aanvoertarget bepaalt *hoe warm* het water moet zijn. Voorbeeld buffervat: onder 38 °C buffer­temperatuur `Heating Enable` aan met een aanvoertarget van 42 °C, boven 40 °C `Heating Enable` uit.

Kijk bij twijfel naar `Heating Supply – target source`. Die staat op `external` zolang het externe target echt gebruikt wordt, en op `curve` zodra de stooklijn weer rekent. `Heating Curve Supply Target` blijft altijd het lokale stooklijnreferentie­doel tonen, ook terwijl een extern target actief is.

## Regelprofiel Buffer (groot buffervat)

Regelt OpenQuatt op de temperatuur van een groot buffervat (bijvoorbeeld 1000 liter) in plaats van op de directe aanvoer, dan reageert de gemeten temperatuur zo traag dat de gewone profielen `Comfort`, `Balanced` en `Stable` te snel stoppen en daarna met veel vermogen herstarten. Kies in dat geval onder `Heating Curve Control Profile` het profiel `Buffer`. De andere profielen veranderen hierdoor niet.

Wat `Buffer` anders doet:

- **Doordraaien op het laagste niveau:** zolang de warmtevraag actief is en de aanvoer onder `doel + 1 °C` blijft, vraagt OpenQuatt minimaal niveau 1 van één unit. De warmtepomp valt dus niet stil zodra de PID-uitvoer rond het doel nul wordt.
- **Minimaal vermogen instelbaar:** met `Buffer Minimum Demand` (schaal 1 tot 10, standaard 1) kies je hoe hoog dat minimum ligt. Is de warmtevraag van het huis structureel hoger dan één unit op niveau 1 (ongeveer 2,5 kW), zet de waarde dan hoger. Bij ongeveer 5 draait één unit op niveau 2 (circa 3 kW), bij 6 tot 7 op niveau 3 (circa 3,6 kW) en bij ongeveer 8 draaien beide units op niveau 1. De precieze niveaus hangen af van buitentemperatuur en aanvoerdoel; begin voorzichtig en bekijk het verloop van het vat.
- **Geleidelijk terugzakken:** een verhoogd minimum geldt tot het doel. Tussen het doel en `doel + 1 °C` zakt het lineair terug naar niveau 1, zodat een opwarmend vat de units stapsgewijs afbouwt (bijvoorbeeld niveau 3, 2, 1) voordat de stop volgt. Zakt het vat weer, dan loopt het minimum op dezelfde manier terug omhoog.
- **Geen laaglast-stop:** de vroege stop vlak boven het doel is uitgeschakeld.
- **Stoppen pas 1 °C boven het doel:** de aanvoer moet 5 minuten aaneengesloten op `doel + 1 °C` of hoger zitten, en de PID mag geen noemenswaardige vraag meer hebben. Een positieve PID-integraal kan het stoppen daarom een paar minuten uitstellen. In die bevestigingstijd blijft het laagste niveau actief.
- **Herstart bij 1,5 °C tekort:** na een echte stop start OpenQuatt weer als de aanvoer 1,5 °C onder het doel zakt (na minimaal 5 minuten uit). Dat is een gewone MAINTAIN-start met oplopend vermogen, geen RECOVERY.
- **RECOVERY pas bij een groot tekort:** vanaf 2 °C onder het doel, terug naar MAINTAIN bij 0,8 °C. Gewone dalingen worden daardoor opgevangen met het afgeremde, oplopende vermogen van MAINTAIN.
- **Rustige niveaustappen:** omhoog elke 120 seconden (60 seconden in RECOVERY), omlaag elke 30 seconden.
- **Tweede unit later:** bij `Duo` mag de tweede unit pas 15 minuten na de start van de eerste bijspringen.

Let op:

- Het profiel gaat uit van een buffer die een flink deel van de warmtevraag overbrugt. Met een klein vat of een warmtevraag onder het minimale vermogen van één unit blijft het pendelen groter dan hier beschreven.
- Heeft de installatie een kamertemperatuur die vers en geldig is, dan stopt OpenQuatt alleen als de kamer niet kouder is dan het setpoint plus de warme grens, zoals bij de andere profielen. De buffer kan dan boven `doel + 1 °C` uitkomen; de aanvoer blijft begrensd door `Maximum water temperature`.
- Bij `Duo` met `Share Load` springt de tweede unit bij een tekort vanaf ongeveer 0,5 °C bij als `Share Load Start Level` laag staat. Een hoger startniveau (bijvoorbeeld 4 of 5) houdt één unit langer alleen aan het werk; bekijk eerst de niveaus en starts per uur voordat je dit aanpast.
- Een wijziging van het profiel zet de PID-integraal terug en start de regeling opnieuw.

## Welke instellingen zijn voor de meeste gebruikers het belangrijkst?

Als je deze strategie afstelt, begin dan bijna altijd hier:

1. de zes stooklijnpunten
2. `Curve Fallback Tsupply (No Outside Temp)`
3. `Heating Curve PID Kp`
4. `Heating Curve PID Ki`
5. `Maximum water temperature`

Bij `Duo` daarna eventueel:

6. `Minimum runtime`
7. `Duo Dispatch Mode`
8. `Share Load Start Level` (alleen relevant bij `Share Load`)
9. `Dual HP Enable Hold`
10. `Dual HP Disable Hold`

## Wat merk je bij Single en Duo?

### Single

Bij `Single` is het gedrag vrij rechtlijnig:

- stooklijn bepaalt het doel;
- PID stuurt de aanvoer;
- de compressor volgt die vraag binnen de normale beveiligingen.

### Duo

Bij `Duo` werkt deze strategie eenvoudiger dan bij `Power House`. Met `Duo Dispatch Mode` kies je hoe de tweede unit erbij komt:

#### 1 Running / 1 Standby (standaard)

- OpenQuatt begint normaal met één warmtepomp;
- de tweede unit komt pas bij als de vraag hoog genoeg en lang genoeg aanwezig blijft;
- loopt de draaiende warmtepomp nog op, dan komt de tweede unit pas bij zodra die warmtepomp volgens het prestatiemodel minstens zoveel levert als de kleinste Duo-combinatie; zo springt de tweede unit niet bij om kort daarna weer te stoppen. Kan de draaiende warmtepomp niet verder oplopen, bijvoorbeeld door `Silent max frequency`, een actieve storing of ontdooien, dan geldt deze wachttijd niet;
- er zit dus bewust wachttijd en hysterese in.

Dat voorkomt onrustig schakelen tussen één en twee warmtepompen. Dit is en blijft het standaardgedrag; een bestaande Duo-installatie merkt niets van deze instelling zolang ze niet zelf wordt gewijzigd.

#### Share Load

- OpenQuatt begint ook hier met één warmtepomp, maar laat die nooit verder oplopen dan `Share Load Start Level` (standaard niveau 2, instelbaar van 2 tot 9);
- is er meer vermogen nodig dan die ene unit op dat niveau kan leveren, dan start de tweede unit erbij in plaats van de eerste unit verder op te voeren — bijvoorbeeld unit 1 op niveau 2 en unit 2 op niveau 1;
- vraagt het nog meer, dan lopen beide units daarna gelijk op (bijvoorbeeld 2+2, dan 3+2, dan 3+3, enzovoort) in plaats van dat één unit alleen doorgroeit naar zijn maximum. De twee units blijven daarbij maximaal één niveau uit elkaar (dus niet 5+1 maar 3+3). Zolang het water ver onder de aanvoertarget zit, mikt OpenQuatt daarbij op het vermogen van beide units samen, zodat ze doorlopen tot de aanvoertarget bereikt is of ze niet harder kunnen;
- welke unit als eerste (de "lead") start, wisselt vanzelf mee met de al opgebouwde looptijd van beide units — er is dus geen vaste "unit 1 start altijd eerst".

`Share Load` is bedoeld voor wie liever het vermogen vroeg over beide units verdeelt (bijvoorbeeld voor een rustiger geluidsbeeld of gelijkmatiger slijtage) in plaats van één unit hoog te laten oplopen voordat de tweede meedoet. `Dual HP Enable Hold` en `Dual HP Disable Hold` blijven in beide modi de bescherming tegen snel heen-en-weer schakelen.

## Wat hoef je meestal niet meteen aan te raken?

Voor de meeste gebruikers zijn dit geen eerste knoppen:

- ingewikkelde lage-vraag faselogica;
- alle details van `HEAT`, `COAST` en `OFF`;
- een hoge `Kd` om kleine problemen "weg te dempen";
- meerdere stooklijnpunten en PID-waarden tegelijk.

Begin bijna altijd met de stooklijn zelf. Pas daarna komt PID.

## Handige meetwaarden om naar te kijken

Als deze strategie niet logisch voelt, kijk dan eerst naar:

- `Outside Temperature (Selected)`
- `Water Supply Temp (Selected)`
- `Heating Curve Supply Target`
- `Heating Supply Target (Selected)`
- `Heating Supply – target source`
- de actieve `Heating Control Mode`

Controleer daarna pas of de PID of `Duo`-grenzen te scherp staan.

## Veilige volgorde van afstellen

1. Controleer eerst of de gekozen buitentemperatuur en aanvoerbron kloppen.
2. Maak daarna de stooklijn logisch.
3. Pas daarna pas `Kp` en `Ki` aan.
4. Kijk bij `Duo` pas als laatste naar de inschakel- en wachttijden.
5. Verander steeds maar één groep tegelijk.

## Veelvoorkomende misverstanden

- Een goede stooklijn maakt PID niet overbodig.
- Meer `Kp` is niet automatisch beter.
- `Duo` kiest hier niet dezelfde logica als in `Power House`.
- Een rustige `COAST`-fase betekent niet automatisch dat er iets mis is.

## Verder lezen

- [Verwarmen en comfort](dagelijks/verwarmen.md)
- [Power House](power-house.md)
- [Regelgedrag van OpenQuatt](regelgedrag-van-openquatt.md)
- [Instellingen en meetwaarden](instellingen-en-meetwaarden.md)
- [Problemen oplossen](problemen-oplossen.md)
- [Verwarmingsregeling voor ontwikkelaars](development/verwarmingsregeling.md)
