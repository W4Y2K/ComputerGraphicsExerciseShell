Team: 7269351, 9173456, 6665514

Uebersicht Punkte:
vermutlich 50pkt Mindestanforderung
- 5 Punkte: Skybox (fertig)
- 5 Punkte: Point Lights (fertig)
- 5 Punkte: Camera Modes (Fov ... muss noch wieder implementiert werden)
- 5 Punkte: Animation Nodes
- 10 Punkte: B-Spline Camera
- 10 Punkte: Spline Mesh
- 10 Punkte: Spline UI 



6 Mindest Anforderungen:
6.1 Rasterisation Pipeline
Implementiere eine Rasterisation Rendering Pipepline mit OpenGL um Ge-
omtrie darzustellen.


6.2 Scene Graph
Der Prototype muss einen Scene Graph zum Rendern verwenden. In diesem
muss auch die zum rendern verwendet Camera befinden.

6.3 Model Loading
Der Prototype muss 3D-Modelle laden und rendern koeen. 

6.4 Phong Shading
Der Prototype muss die 3D Modelle mit dem Phong Beleuchtungsmodell anzeigen
koennen

6.5 Orbital Camera
Der Nutzer muss in der Lage sein, die Camera um die 3D modelle zu rotieren.

6.6 User Interface
Der Prototype muss ein Userinterface bereitstellen. 

6.7 Dokumentation
Das Projekt muss dokumnetiert werden. Sowohl mit Kommentaren in Code, als
auch mit einem README Document, dass die erf¨ullten optionalen Aufgaben
beschreibt. Wird das Projekt als Team bearbeitet, muss zudem die Aufgaben-
verteilung dokumentiert werden. Insbesondere wer and welchen optionalen Auf-
gaben gearbeitet hat. Einen ¨Uberblick ¨uber das gesamte Projekt wird von jedem
Teammitglied erwartet, unabh¨angig davon welche Aufgaben prim¨ar bearbeitet
wurden.


Optionale Aufgaben:

7.1.4 Cube Map Skybox (5P)(7269351)
Erstellen Sie eine Skybox basierend auf einer Cube Map, um eine realistische
Umgebungsdarstellung zu erm¨oglichen. Verwenden Sie hochaufl¨osende Texturen
f¨ur eine ¨uberzeugende Hintergrunddarstellung. Sie k¨onnen die Skybox auch als
Lichtquelle f¨ur den Ambienten Anteil ihrer Beleuchtung verwenden.

7.1.3 Point Lights (5P)(9173456)
F¨ugen Sie Punktlichter hinzu, um realistische Beleuchtungseffekte zu erzeugen.
Ber¨ucksichtigen Sie Lichtausbreitung f¨ur eine verbesserte Darstellung. Schat-
ten sind Teil der der Optionalen Aufgabe 8.7 und d¨urfen f¨ur diese Aufgabe
vernachl¨assigt werden

7.3.1 Camera Modes (5P)(7269351)
Implementieren Sie verschiedene weitere Kameramodi, wie eine 6Degrees-of-
Freedom-Camera (Horizontale, Verticale, Laterale-Bewegung, sowie Roll, Pitch
und Yaw), um die Navigation innerhalb der Szene zu verbessern. F¨ugen sie
zudem Optionen hinzu das FieldOfView und die Far- und Near-Clipping plane
im GUI einzustellen.

7.3.2 Animation Nodes (5P)(7269351)
F¨ugen sie verschiedene Animation Nodes hinzu um die Scene zubewegen:
 Stetige Rotation um eine Axse
 Ozilierende Rotation um eine Axse
 Ozilierende Translation entlang einer Axse

7.4.1 Kamerafahrt mit B-Splines (10P)(9173456)
F¨ugen sie eine Kamerafahrt durch die Scene hinzu. Diese sollte entweder von
alleine starten und sich wiederholen oder durch einen Tastendruck abgespielt
werden k¨onnen. Alternativ k¨onnen sie auch andere Objekt eintlang einer B-
Spline animieren.

7.4.2 Spline Mesh (10P)(6665514)
Implementieren Sie ein Rendering-System f¨ur Splines. Erstellen sie ein 2D oder
3D mesh entlang einer spline. Zur Evaluierung der einezlen Kurven k¨onnen sie
funktionen aus glm :: gtx :: spline benutzen.

7.4.3 Spline UI (10P)(6665514)
Erstellen Sie eine interaktive Benutzeroberfl¨ache zur Steuerung von Splines.
Erm¨oglichen Sie Nutzern das Erstellen, Bearbeiten und Anpassen von Kurven
innerhalb der Szene. Die erstellten Splines sollten speicherbar sein und wieder
geladen werden koennen