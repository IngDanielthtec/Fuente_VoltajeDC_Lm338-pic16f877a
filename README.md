# Fuente ajustable LM338 con PIC16F877A

Firmware en C (mikroC) para una fuente de voltaje ajustable de **2 a 20 VDC**.
El voltaje se ingresa por teclado matricial, se fija con un potenciómetro
digital hecho de resistencias binarias y optoaisladores, y se mide con el ADC
del PIC para mostrarlo en un LCD 2x16 y corregirlo en lazo cerrado.

## Características

- Entrada por teclado: 1 o 2 dígitos (`5`, `12`) y `#` para confirmar, `*` para reiniciar.
- Rango validado de 2 a 20 V; fuera de rango muestra `Ingrese 2-20VDC`.
- Potenciómetro digital de 8 bits (pesos 1X…128X) con optoaisladores y CD4066B.
- Lectura de Vsalida con promedio de 16 muestras, refrescada cada ~0.4 s.
- Ajuste en lazo cerrado opcional (±1 escalón por paso, máx. 6) que compensa
  VCE(sat) de los optos y tolerancias de resistencias.
- Arranque seguro: `PORTC=255` (resistencia mínima) antes de habilitar la salida.

## Hardware

| Bloque | Componente |
|---|---|
| Regulador | LM338K (referencia del PIC: LM317LZ) |
| Microcontrolador | PIC16F877A, cristal 4 MHz |
| Pantalla | LCD 2x16, 4 bits, RW a GND |
| Teclado | Matricial 4x4 (sin usar la columna A–D) con pull-down |
| Potenciómetro digital | 8 resistencias R1…R8 + 8 optoaisladores |
| Otros | CMOS CD4066B |

Conexiones del firmware:

| Puerto | Función |
|---|---|
| RA0 / AN0 | Sensor de voltaje (divisor R9 + X2) |
| RA3 / AN3 | Vref+ (LM317, ajuste con X3) |
| PORTB | LCD |
| PORTC | Optoaisladores (bit=1 cortocircuita la resistencia) |
| PORTD | Teclado |

Más detalle en [`docs/hardware.md`](docs/hardware.md).

## Teclado

| Tecla | Valor devuelto | Función |
|---|---|---|
| `0`–`9` | 6–12, 14–16 | Dígitos |
| `#` | 5 | Enter |
| `*` | 13 | Reset |
| Columna `A`–`D` | 1–4 | Sin uso |

## Compilación

**mikroC clásico** (sintaxis original): abrir un proyecto PIC16F877A a 4 MHz,
agregar `src/main.c` y compilar. El LCD usa el mapeo por defecto de
`Lcd_Init(&PORTB)`: verifica que coincida con tu esquema.

**mikroC PRO for PIC**: agrega antes de `main()` y cambia las llamadas:

```c
sbit LCD_RS at RB2_bit;  sbit LCD_EN at RB3_bit;
sbit LCD_D4 at RB4_bit;  sbit LCD_D5 at RB5_bit;
sbit LCD_D6 at RB6_bit;  sbit LCD_D7 at RB7_bit;
sbit LCD_RS_Direction at TRISB2_bit;
sbit LCD_EN_Direction at TRISB3_bit;
sbit LCD_D4_Direction at TRISB4_bit;
sbit LCD_D5_Direction at TRISB5_bit;
sbit LCD_D6_Direction at TRISB6_bit;
sbit LCD_D7_Direction at TRISB7_bit;
char keypadPort at PORTD;
```

y usa `Lcd_Init();`, `Keypad_Init();`, `_LCD_CLEAR` y `_LCD_CURSOR_OFF`.
(El mapeo de pines del LCD está leído del esquema; verifícalo en tu placa.)

**Config bits:** oscilador XT, WDT off, PWRT on, **LVP off**.
Si `ADC_Init()` no existe en tu versión, bórralo.

## Calibración (obligatoria)

1. Ajusta X3 para que Vref+ (RA3) esté entre ~2.5 V y 5 V, sin superar VDD.
2. Ajusta X2 para que con 20 V reales en la salida el ADC lea 1023.
3. Compara la salida real con el multímetro en 2, 5, 10, 15 y 20 V y ajusta la
   tabla en `main.c` si hace falta.

Pasos detallados en [`docs/calibracion.md`](docs/calibracion.md).
Si no puedes calibrar, comenta `#define LAZO_CERRADO` para usar solo la tabla.

## Puntos a verificar en el hardware

- **Suma de pesos:** 1X+…+128X = 255X (no 256X). Con 1X ≈ 26.92 Ω el máximo
  teórico es ≈ 19.85 V; para 20 V puede hacer falta `PORTC=0`.
- **R8 = 470 Ω** da ~2.7 mA de corriente de referencia, por debajo del mínimo
  típico del LM338 (~3.5 mA). Con 240 Ω es el valor habitual (hay que recalcular la tabla).
- **Corriente de los LED de los optos:** 5 V / 220 Ω ≈ 22.7 mA por pin; con los
  8 encendidos queda cerca del límite de puerto del PIC. Considera 330 Ω.
- **Pines:** confirma que optos van a PORTC y teclado a PORTD en tu placa.

## Estructura

```
.
├── README.md
├── LICENSE
├── .gitignore
├── src/
│   └── main.c
└── docs/
    ├── hardware.md
    ├── calibracion.md
    └── tabla_codigos.md
```

## Licencia

MIT. Ver [`LICENSE`](LICENSE).
