# BloomDB – Roadmap

## ✅ Fase 1 – Core (COMPLETADA)
- [x] Bloom filter estándar en C
- [x] Arquitectura modular básica (bloomdb / bitarray / hash64 / storage)
- [x] Persistencia binaria simple (.bloomdb)
- [x] Ejemplo en `main.c`
- [x] Makefile básico
- [x] Tests unitarios básicos (bitarray, hash64, bloomdb, storage)
- [x] Tests de memoria (Valgrind + ASan)
- [x] Tests de Rendimiento ()
- [x] Benchmarks profesionales con percentiles y exportación JSON

## ✅ Fase 2 – Librería sólida (COMPLETADA)
- [x] Tests unitarios por módulo (test_bloomdb_ex, test_storage_ex, test_helpers)
- [x] Manejo de errores más explícito (BloomDBError enum con 5 códigos)
- [x] API helpers (bloomdb_*_cstr, bloomdb_*_u64)
- [x] Funciones *_ex con retorno de error explícito
- [x] API simple como wrappers de funciones *_ex
- [x] Documentación API completa (docs/API_REFERENCE.md)

## Fase 3 – Variantes de Bloom Filters
- [ ] Blocked Bloom filter
- [ ] Scalable Bloom filter
- [ ] Counting Bloom filter
- [ ] Partitioned Bloom filter

## Fase 4 – Persistencia avanzada
- [ ] Snapshots periódicos
- [ ] Append-only log
- [ ] Recovery al iniciar
- [ ] Formato de "instancia" en disco (similar a una DB)

## Fase 5 – Optimización extrema
- [ ] Benchmarks de inserción/consulta
- [ ] Implementación branchless de bitarray
- [ ] Prefetching de memoria en los hot paths
- [ ] Implementaciones opcionales con AVX2/AVX-512 (intrinsics)
- [ ] Opcional: versiones en ensamblador para funciones críticas

## Fase 6 – Servidor network
- [ ] Protocolo binario simple (request/response)
- [ ] Loop de eventos con epoll/kqueue
- [ ] Pipelining
- [ ] Soporte multi-cliente

## Fase 7 – Clustering y SDKs
- [ ] Sharding y consistent hashing
- [ ] Replicación
- [ ] Cliente/SDK para Node.js
- [ ] Cliente/SDK para Python
- [ ] Cliente/SDK para Go
- [ ] Cliente/SDK para Rust
