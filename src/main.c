/*****************************************************************
  Fuente ajustable LM338 (2 - 20 VDC) - PIC16F877A - mikroC
  Con ajuste en lazo cerrado (corrige VCE(sat) y tolerancias)

  PORTA.0 (AN0) : sensor de voltaje (divisor R9/X2)
  PORTA.3 (AN3) : Vref+ (X3)
  PORTB         : LCD 2x16, 4 bits
  PORTC         : optoaisladores (bit=1 -> cortocircuita la resistencia)
  PORTD         : teclado matricial

  Teclas: 5='#' ENTER   13='*' RESET   (1..4 sin uso)

  Config bits: XT, 4 MHz, WDT off, PWRT on, LVP OFF

  IMPORTANTE: calibrar X3 y X2 para que 20 V reales = 1023 cuentas
  antes de usar el lazo cerrado (ver docs/calibracion.md).
  Si no, comenta LAZO_CERRADO.
*****************************************************************/

#define LAZO_CERRADO          // comenta esta linea para desactivar el ajuste
#define TOL_MV      35        // tolerancia (medio escalon ~ 36 mV)
#define MAX_PASOS   6         // maximo de escalones de correccion

// PORTC para 2..20 V (valores iniciales, ya con VCE(sat))
const unsigned short tabla[19] = {
  255,240,227,215,200,191,175,159,144,131,
  120,107, 95, 79, 64, 48, 33, 20, 11
};

unsigned short keyp, contador, d1, d2, result, ciclo;
char keyput;

// ---------------------------------------------------------------
// Devuelve Vsalida en mV (promedio de 16 muestras)
// ---------------------------------------------------------------
unsigned long medir_mv(void) {
  unsigned long suma, mv;
  unsigned short i;
  unsigned int adc;

  suma = 0;
  for (i = 0; i < 16; i++) {
    adc = ADC_Read(0);
    suma = suma + adc;
    Delay_ms(2);
  }
  suma = suma / 16;
  mv = suma;
  mv = mv * 20000;
  mv = mv / 1023;
  return mv;
}

// ---------------------------------------------------------------
// Muestra mv en fila 1, columnas 10..15: "12.34V"
// ---------------------------------------------------------------
void mostrar_mv(unsigned long mv) {
  unsigned short ch;

  ch = (mv / 10000) % 10;
  if (ch == 0) Lcd_Chr(1, 10, ' ');
  else         Lcd_Chr(1, 10, 48 + ch);
  ch = (mv / 1000) % 10;
  Lcd_Chr(1, 11, 48 + ch);
  Lcd_Chr(1, 12, '.');
  ch = (mv / 100) % 10;
  Lcd_Chr(1, 13, 48 + ch);
  ch = (mv / 10) % 10;
  Lcd_Chr(1, 14, 48 + ch);
  Lcd_Chr(1, 15, 'V');
}

void mostrar_vsalida(void) {
  mostrar_mv(medir_mv());
}

// ---------------------------------------------------------------
void pantalla_inicial(void) {
  Lcd_Cmd(Lcd_Clear);
  Delay_ms(5);
  Lcd_Out(1, 1, "Vsalida: ");
  Lcd_Out(2, 1, "Ventrada: ");
  mostrar_vsalida();
}

// ---------------------------------------------------------------
// Aplica el voltaje pedido (2..20) y lo corrige con el ADC
// Mayor valor de PORTC = menos resistencia = menos voltaje
// ---------------------------------------------------------------
void ajustar(unsigned short v) {
  unsigned short code, lastcode, pasos;
  unsigned long objetivo, mv, err, errprev;

  code = tabla[v - 2];
  PORTC = code;
  Delay_ms(200);                       // asentamiento (C5 = 33 uF)

#ifdef LAZO_CERRADO
  objetivo = v;
  objetivo = objetivo * 1000;
  errprev = 65535;
  lastcode = code;
  pasos = 0;

  Lcd_Out(1, 1, "Ajustando...    ");

  while (pasos < MAX_PASOS) {
    mv = medir_mv();
    if (mv > objetivo) err = mv - objetivo;
    else               err = objetivo - mv;

    if (err <= TOL_MV) break;          // dentro de tolerancia

    if (err > errprev) {               // empeoro: volver al mejor
      code = lastcode;
      PORTC = code;
      Delay_ms(200);
      break;
    }
    errprev = err;
    lastcode = code;

    if (mv < objetivo) {               // falta voltaje: menos codigo
      if (code == 0) break;
      code = code - 1;
    }
    else {                             // sobra voltaje: mas codigo
      if (code == 255) break;
      code = code + 1;
    }
    PORTC = code;
    pasos++;
    Delay_ms(200);
  }
  Lcd_Out(1, 1, "Vsalida: ");
#endif

  mostrar_vsalida();
}

// ---------------------------------------------------------------
void main() {
  PORTB = 0;
  PORTC = 255;               // minimo voltaje ANTES de poner TRISC como salida
  TRISB = 0x00;
  TRISC = 0x00;
  TRISD = 0x0F;
  TRISA = 0xFF;

  Keypad_Init(&PORTD);
  Lcd_Init(&PORTB);
  Lcd_Cmd(Lcd_Clear);
  Lcd_Cmd(Lcd_Cursor_Off);

  ADC_Init();                // borrar si tu compilador no lo tiene
  ADCON1 = 0b10001000;       // derecha, Vref+ en RA3

  contador = 0; d1 = 0; d2 = 0; result = 0; ciclo = 0;

  Delay_ms(200);
  pantalla_inicial();

  while (1) {

    keyp = Keypad_Released();

    if (keyp == 0) {                   // sin tecla: refrescar medicion
      Delay_ms(5);
      ciclo++;
      if (ciclo >= 60) {
        ciclo = 0;
        mostrar_vsalida();
      }
      continue;
    }

    switch (keyp) {
      case 5:  keyput = 'E'; break;    // ENTER (#)
      case 6:  keyput = '9'; break;
      case 7:  keyput = '6'; break;
      case 8:  keyput = '3'; break;
      case 9:  keyput = '0'; break;
      case 10: keyput = '8'; break;
      case 11: keyput = '5'; break;
      case 12: keyput = '2'; break;
      case 13: keyput = 'R'; break;    // RESET (*)
      case 14: keyput = '7'; break;
      case 15: keyput = '4'; break;
      case 16: keyput = '1'; break;
      default: keyput = 0;   break;
    }
    if (keyput == 0) continue;

    // ---------------- RESET ----------------
    if (keyput == 'R') {
      Lcd_Cmd(Lcd_Clear);
      Delay_ms(200);
      Lcd_Out(1, 1, "reset....");
      Delay_ms(500);
      contador = 0; d1 = 0; d2 = 0;
      pantalla_inicial();
      continue;
    }

    // ---------------- ENTER ----------------
    if (keyput == 'E') {
      if (contador == 0) continue;

      if (contador == 1) result = d1;
      else               result = d1 * 10 + d2;
      contador = 0; d1 = 0; d2 = 0;

      if (result > 20 || result < 2) {
        Lcd_Cmd(Lcd_Clear);
        Delay_ms(200);
        Lcd_Out(1, 8, "!!!");
        Lcd_Out(2, 1, "Ingrese 2-20VDC");
        Delay_ms(1000);
        pantalla_inicial();
      }
      else {
        ajustar(result);
      }
      continue;
    }

    // ---------------- DIGITO ----------------
    if (contador < 2) {
      if (contador == 0) {
        Lcd_Out(2, 1, "Ventrada:      ");
        d1 = keyput - '0';
      }
      else {
        d2 = keyput - '0';
      }
      Lcd_Chr(2, 11 + contador, keyput);
      contador++;
      Lcd_Chr(2, 11 + contador, 'V');
    }
  }
}
