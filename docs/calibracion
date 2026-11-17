# Calibración

Equipo: multímetro y una carga resistiva ligera (≥ 10 mA para el LM338).

1. **Vref del ADC (X3).** Mide RA3. Debe estar entre ~2.5 V y 5 V, estable y
   sin superar VDD.
2. **Escala del ADC (X2).** Fija 20 V en la salida (con la tabla o `PORTC` manual)
   y ajusta X2 hasta que el LCD muestre 20.00 V igual al multímetro.
   Con el divisor bien ajustado, 20 V = 1023 cuentas.
3. **Verificación.** Pide 2, 5, 10, 15 y 20 V. Anota salida real (multímetro)
   y lectura del LCD. Deben coincidir dentro de ±0.1 V.
4. **Tabla.** Si hay desvíos grandes con `LAZO_CERRADO` comentado, corrige los
   valores de `tabla[]` en `src/main.c`.
5. **Lazo cerrado.** Con `LAZO_CERRADO` activo, el firmware corrige hasta
   ±6 escalones (~±430 mV) usando la lectura del ADC. Una lectura mal
   calibrada desplaza la salida real, así que haz los pasos 1–3 primero.

Constantes ajustables en `main.c`:

| Constante | Valor | Función |
|---|---|---|
| `TOL_MV` | 35 | Tolerancia de aceptación (mV) |
| `MAX_PASOS` | 6 | Máximo de escalones de corrección |
