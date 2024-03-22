# Consola de Comandos FAT16

Este proyecto consiste en una aplicación de consola desarrollada en C++ que permite la manipulación de imágenes en formato FAT16. La herramienta proporciona una serie de comandos que permitan realizar diversas operaciones sobre la imagen, como la creación, lectura, escritura y eliminación de archivos.

## Requisitos
- CMake 3.27.8 o mayor 
- Un archivo formateado con el sistema de archivos FAT16

## Instalación

### Clonar el repositorio

```bash
git clone https://github.com/SamuelRocca85/FAT16Console.git 
```

### Compilar el proyecto con CMake 

```bash
# Crear la carpeta de compilación
mkdir build
# Generar archivos con cmake 
cmake -S . -B ./build
# Compilar 
cmake --build ./build
```

## USO

El programa debe recibir como argumento el archivo que contiene la imagen FAT16 valida.

```bash 
# Ejecución en Linux
./build/fatconsole ./images/fat.img
```

El programa mostrara una consola donde se pueden ingresar los siguientes comandos.

|Comando|Uso|Ejemplo|
|-------|---|-------|
|ls|Muestra los directorios y archivos en la ubicación de la consola.|ls|
|cd [dir]|Cambia el directorio actual por el especificado como [dir]|cd TEST|
|mkdir [dir]|Crea un directorio nuevo con nombre [dir] en el directorio actual|mkdir TEST|
cat > [file]|Crea un archivo con nombre [file] y espera el input para escribir en el archivo|cat > test.txt|
cat [file]|Muestra el contenido del archivo [file]|cat test.txt|

> [!NOTE]  
> Los nombres de los directorios en el comando mkdir deben ingresarse con mayusculas sin espacios
