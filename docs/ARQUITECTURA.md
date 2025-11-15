# BloomDB – Arquitectura de la Fase 1

## 1. Visión general

BloomDB es una implementación en C de un **Bloom Filter**: una estructura de datos probabilística que permite responder de forma muy rápida a la pregunta:

> “¿Este elemento podría estar en el conjunto?”

Propiedades clave:

- Nunca da **falsos negativos**: si dice que NO está, de verdad no está.
- Puede dar **falsos positivos**: a veces dice que está, pero en realidad no.
- Usa muy poca memoria, representando el conjunto como un **arreglo de bits**.

En esta Fase 1, BloomDB está pensada como una **biblioteca simple, single-threaded**, con:

- inserción de claves (`bloomdb_insert`)
- consulta (`bloomdb_might_contain`)
- guardado/carga en disco (`bloomdb_save` / `bloomdb_load`)

---

## 2. Estructura de archivos

```text
bloomdb/
│
├── include/                 # Headers públicos
│   ├── bloomdb.h           # API principal
│   ├── bitarray.h          # Operaciones sobre arreglos de bits
│   ├── hash64.h            # Función hash de 64 bits
│   └── storage.h           # Persistencia (save/load)
│
├── src/                    # Implementaciones
│   ├── bloomdb.c           # Lógica del Bloom Filter
│   ├── bitarray.c          # Set/get de bits en el bitarray
│   ├── hash64.c            # Hash base + mixing
│   └── storage.c           # Guardado y carga binaria
│
├── main.c                  # Programa de ejemplo y prueba manual
└── Makefile                # Script de compilación
