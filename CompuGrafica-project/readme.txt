==========================================================
 AUTODROMO HERMANOS RODRIGUEZ (F1)  -  Proyecto Final
 Computacion Grafica e Interaccion Humano-Computadora
 Facultad de Ingenieria, UNAM  -  Semestre 2027-1
 Tema: Mexico y su cultura
 Equipo: La Rata de Emi y las Nutrias
 Autores: Cruz Basilio Ximena Carolina, Jardon Marin Rodrigo,
          Jimenez Olivo Evelin, Parra Fernandez Hector Emilio
==========================================================

REQUISITOS
 - Windows 10/11 (el proyecto se compila en x86 / Release y corre en
   sistemas de 32 y 64 bits)
 - Tarjeta grafica compatible con OpenGL 3.3 o superior
 - Visual C++ Redistributable 2015-2022 (x86). Si al abrir el juego
   aparece un error de VCRUNTIME140.dll o MSVCP140.dll, instalarlo desde
   la pagina de Microsoft.

INSTALACION
 1. Ejecutar Instalador_ProyectoF1.exe (doble clic).
 2. Aceptar la carpeta de instalacion que propone (C:\ProyectoF1) o
    elegir otra. Se recomienda una carpeta fuera de OneDrive.
 3. Terminar el asistente. Se crea un acceso directo en el Escritorio
    y en el Menu Inicio.
 4. Abrir el juego con el acceso directo "ProyectoF1" o ejecutando
    CompuGrafica-project.exe desde la carpeta de instalacion.
    Las carpetas Models, images, Shader y audio deben estar junto al .exe
    (el instalador las deja ahi).
 Nota: si Windows Defender o el antivirus bloquea el .exe (programa sin
 firmar), elegir "Mas informacion" -> "Ejecutar de todas formas", o permitirlo
 en Seguridad de Windows.

USO
 Al abrir, el carro espera en la meta. Presionar W para arrancar y dar
 la vuelta al circuito. Cambiar de camara con C.

CONTROLES
 Carro (modo carrera)
   W / flecha arriba     acelerar
   S / flecha abajo      frenar y reversa
   A, D / flechas        girar
   R                     abrir / cerrar el DRS
   T                     reiniciar la vuelta
   B                     pedir parada en boxes
 Escena
   C                     cambiar de camara (tercera persona, cabina, libre)
   Camara libre          W A S D mover, Shift rapido, mouse para mirar
   F                     encender / apagar los postes de luz
   N                     cambiar dia / atardecer / noche
   L                     ver la linea (spline) de la pista
   P                     abrir / cerrar el podio
   ESC                   salir
 Video / recorrido
   V                     recorrido automatico de camara por todo el
                         proyecto (dia, atardecer y noche); V de nuevo lo cancela
   J                     camara que gira alrededor de las 6 personas animadas
                         de la grada; J de nuevo la cancela
 Animaciones por huesos
   K                     el alebrije aletea / descansa
   G                     el publico de la grada nueva festeja / se detiene
   H                     Fermin saluda / se detiene
   (el comisario de meta con huesos ondea solo al llegar a la meta)

TECNICAS PRINCIPALES
 - Iluminacion: Blinn-Phong (sol direccional con sombras PCF, 8 spotlights)
   y reflejo Fresnel (Schlick) con mapa de ambiente en la pintura del carro.
 - Skybox / mapa de ambiente: cubemap de 6 caras (images/skybox) combinado
   con el cielo procedural dia / atardecer / noche.
 - Animacion por huesos (skinning): alebrije, publico y comisario/Fermin.
 - Animacion por keyframes (DRS, bandera del comisario, Fermin) y por codigo
   (ruedas, confeti, derrape del carro del podio, calaveras, bandera y papel
   picado con el viento, publico de munecos cuadrados en el vertex shader).
 - Modelos creados en Blender: publico, comisario/Fermin, alebrije y arco de
   cempasuchil detras del altar de muertos.

CREDITOS Y ENLACES
 Modelos propios y de biblioteca: ver el reporte (DocumentacionFinal.pdf).
 Codigo fuente: https://github.com/evelin-jimenez1/ProyectoLabF1
 Video demostrativo: ver el enlace en el reporte y en la carpeta
 Video_Demostrativo del repositorio.
