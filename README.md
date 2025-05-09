Fragen: 
Cube Map Skybox notwendig oder Skysphere auch ok?
Wie viel noch wegen Kamera Modes?

Uebersicht Punkte:
vermutlich 50pkt Mindestanforderung
- 5 Punkte: Skybox (fertig)
- 5 Punkte: Point Lights (fertig)
- 5 Punkte: Camera Modes (Fov ... muss noch wieder implementiert werden)
- 5 Punkte: Animation Nodes (fertig, Jonas fixt sonnenrotation)
- 10 Punkte: B-Spline Camera (fertig)
- 10 Punkte: Spline Mesh (fertig)
- 10 Punkte: Spline UI (noch nicht implementiert)


Todo:
7.3.1 Camera Modes (5P) 
Implementieren Sie verschiedene weitere Kameramodi, wie eine 6Degrees-of-Freedom-Camera (Horizontale, Vertikale, Laterale Bewegung, sowie Roll, Pitch und Yaw), um die Navigation innerhalb der Szene zu verbessern. Fuegen Sie zudem Optionen hinzu, das FieldOfView und die Far- und Near-Clipping Plane im GUI einzustellen.

7.3.2 Animation Nodes (5P)
Fuegen Sie verschiedene Animation Nodes hinzu, um die Szene zu bewegen:
- Stetige Rotation um eine Achse
- Oszillierende Rotation um eine Achse
- Oszillierende Translation entlang einer Achse

7.4.1 Kamerafahrt mit B-Splines (10P)
F¨ugen sie eine Kamerafahrt durch die Scene hinzu. Diese sollte entweder von
alleine starten und sich wiederholen oder durch einen Tastendruck abgespielt
werden k¨onnen. Alternativ k¨onnen sie auch andere Objekt eintlang einer BSpline animieren.

7.4.2 Spline Mesh (10P)
Implementieren Sie ein Rendering-System f¨ur Splines. Erstellen sie ein 2D oder
3D mesh entlang einer spline. Zur Evaluierung der einezlen Kurven k¨onnen sie
funktionen aus glm :: gtx :: spline benutzen.

7.4.3 Spline UI (10P)
Erstellen Sie eine interaktive Benutzeroberfl¨ache zur Steuerung von Splines.
Erm¨oglichen Sie Nutzern das Erstellen, Bearbeiten und Anpassen von Kurven
innerhalb der Szene. Die erstellten Splines sollten speicherbar sein und wieder
geladen werden k¨onnen