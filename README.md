Fragen: 
Cube Map Skybox notwendig oder Skysphere auch ok?
Wie viel noch wegen Kamera Modes?

Uebersicht Punkte:
vermutlich 50pkt Mindestanforderung
- 5 Punkte: Skybox
- 5 Punkte: Point Lights
- 5 Punkte: Raytracing Spheres
- 5 Punkte: Camera Modes (Fertig)
- 5 Punkte: Animation Nodes (teilweise implementiert), Oszillierende Translation fehlt
- 5 Punkte: Alpha Mask oder Normal Map
- 20 Punkte: Shadows

Todo:
7.2.1 Raytracing Spheres (5P)
Implementieren Sie einen Raytracer zur Darstellung von Kugeln. Berechnen Sie Schnittpunkte entweder in einem FragmentShader oder ComputeShader. Die Schattierung der Kugeln sollte der Qualitaet der Beleuchtung der rasterierten Objekte entsprechen.

7.3.1 Camera Modes (5P) 
Implementieren Sie verschiedene weitere Kameramodi, wie eine 6Degrees-of-Freedom-Camera (Horizontale, Vertikale, Laterale Bewegung, sowie Roll, Pitch und Yaw), um die Navigation innerhalb der Szene zu verbessern. Fuegen Sie zudem Optionen hinzu, das FieldOfView und die Far- und Near-Clipping Plane im GUI einzustellen.

7.3.2 Animation Nodes (5P)
Fuegen Sie verschiedene Animation Nodes hinzu, um die Szene zu bewegen:
- Stetige Rotation um eine Achse
- Oszillierende Rotation um eine Achse
- Oszillierende Translation entlang einer Achse

7.4.1 Spline Renderer (15P)
Implementieren Sie ein Rendering-System fuer Splines, um gekruemmte Linien und Pfade praezise darzustellen. Nutzen Sie mathematische Interpolationstechniken fuer eine exakte Visualisierung.

7.4.2 Spline UI (10P)
Erstellen Sie eine interaktive Benutzeroberflaeche zur Steuerung von Splines. Ermoeglichen Sie Nutzern das Erstellen, Bearbeiten und Anpassen von Kurven innerhalb der Szene. Die erstellten Splines sollten speicherbar sein und wieder geladen werden koennen.

8.5 Shadows (20P)
Implementieren Sie ShadowMapping fuer ihre globale Lichtquelle und/oder Punktlichter.
Guides und Ressourcen:
- Learn OpenGL
- Shadow Mapping - Tutorial

7.1.1 Normal Map (5P)
Implementieren Sie Normal Mapping, um detaillierte Oberflaechenstrukturen ohne zusaetzliche Geometrie darzustellen. Nutzen Sie diese Technik, um realistischere Materialeffekte durch verbesserte Beleuchtung zu erzeugen.

7.1.2 Alpha Mask (5P)
Ermoeglichen Sie Alpha Masking, um Teile einer Textur nicht darzustellen. Nutzen Sie diese Technik fuer Objekte wie Blaetter, Buesche oder Baeume. Es ist ausreichend, Alpha Clipping einzubauen.
