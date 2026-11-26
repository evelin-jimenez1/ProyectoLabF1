==========================================================
 AUTODROMO HERMANOS RODRIGUEZ (F1)  -  Proyecto Final
 Computacion Grafica e Interaccion Humano-Computadora
 Facultad de Ingenieria, UNAM  -  Semestre 2027-1
 Tema: Mexico y su cultura
 Autores: Rodrigo Jardon Marin, Evelin Jimenez Olivo, Ximena Carolina Cruz Basilio, Hector Emilio Parra Fernandez
==========================================================

REQUISITOS
 - Windows 10/11 de 64 bits (el proyecto se compila en x86 / Release)
 - Tarjeta grafica compatible con OpenGL 3.3
 - Ejecutar CompuGrafica-project.exe desde su carpeta: las carpetas
   Models, images y Shader (y los archivos de audio) deben estar junto al .exe.

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
   (ruedas, confeti, derrape del carro del podio, calaveras).

CREDITOS
 Modelos propios y de biblioteca: ver el reporte (DocumentacionFinal.pdf).
