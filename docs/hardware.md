# Hardware

## Principio

El LM338 fija Vout = Vref·(1 + R2/R1) + Iadj·R2, con Vref = 1.25 V, Iadj ≈ 50 µA
y R1 = 470 Ω. R2 es la red binaria controlada por optoaisladores: cada
transistor saturado cortocircuita su resistencia.

- bit = 1 → LED del opto encendido → transistor saturado → resistencia en corto.
- bit = 0 → resistencia en serie.
- Por eso `PORTC` es aproximadamente el complemento del código de la Tabla 3.2.
- Con el PIC sin ejecutar (TRISC como entrada) todos los optos están apagados,
  las resistencias quedan en serie y el voltaje es máximo.

## Pesos de las resistencias

| Peso | Resistencia teórica (Ω) |
|---|---|
| 1X | 26.92 |
| 2X | 53.84 |
| 4X | 107.68 |
| 8X | 215.35 |
| 16X | 430.70 |
| 32X | 861.40 |
| 64X | 1722.81 |
| 128X | 3445.62 |

La suma es 255X ≈ 6865 Ω (la memoria original usa 256X = 6892 Ω).
Para 20 V se requeriría R2 ≈ 7192 Ω según la memoria, por lo que el máximo
teórico queda ligeramente por debajo.

## VCE(sat) de los optos

Cada opto saturado aporta ≈ 0.08 V. Con los 8 saturados son ≈ 0.64 V, que se
suma a la salida mínima (1.25 V + 0.64 V ≈ 1.9 V). Por eso 2 V se obtiene con
`PORTC = 255` y los códigos de `main.c` son unos 250–400 Ω menores que la
Tabla 3.2.

## Alimentación de los LED

`IF = VDD / Rlimitadora = 5 V / 220 Ω ≈ 22.7 mA` (sin descontar la tensión del LED).
Con los 8 encendidos ≈ 180 mA: cerca del límite total de los puertos del PIC
y de los 25 mA por pin.

## LM338

- Corriente de referencia: 1.25 V / 470 Ω ≈ 2.66 mA. El mínimo de carga típico
  del LM338 es ~3.5 mA, por lo que sin carga la salida puede derivar hacia arriba.
- C5 = 33 µF obliga a esperar ~200 ms tras cada cambio antes de medir.

## Config bits del PIC

| Bit | Valor |
|---|---|
| Oscilador | XT (4 MHz) |
| WDT | Off |
| PWRT | On |
| LVP | **Off** (RB3/PGM se usa en el LCD) |
