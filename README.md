# TPActionneurESE



<style>
  .formula {
    font-family: "Courier New", Courier, monospace;
    background-color: #f5f5f5;
    padding: 4px 8px;
    border-radius: 4px;
    display: inline-block;
  }
</style>

- **Fréquence cible** : <span class="formula">f<sub>PWM</sub> = 20,000</span> Hz
- **Horloge du timer** : <span class="formula">f<sub>CLK</sub> = 170</span> MHz
- **Condition supplémentaire** : <span class="formula">ARR > 1024</span>

## Formule de la fréquence PWM en mode aligné au centre

En mode aligné au centre, la fréquence de PWM est calculée par :

<div class="formula">
  f<sub>PWM</sub> = f<sub>CLK</sub> / (2 × PSC × (ARR + 1))
</div>

où :

- <span class="formula">PSC</span> est le prescaler,
- <span class="formula">ARR</span> est le registre de rechargement automatique.

Nous devons trouver des valeurs de <span class="formula">PSC</span> et <span class="formula">ARR</span> qui satisfont cette équation avec les contraintes données.

1. Si <span class="formula">PSC = 2</span>, alors :
   <div class="formula">ARR + 1 = 4250 / 2 = 2125</div>
   Ce qui donne :
   <div class="formula">ARR = 2124</div>

Avec **<span class="formula">PSC = 2</span> et <span class="formula">ARR = 2124</span>**, nous avons une solution qui respecte la condition <span class="formula">ARR > 1024</span>.

## Vérification

Pour vérifier la fréquence obtenue avec <span class="formula">PSC = 2</span> et <span class="formula">ARR = 2124</span>, substituons ces valeurs dans la formule :

<div class="formula">
  f<sub>PWM</sub> = 170,000,000 / (2 × 2 × (2124 + 1))
</div>

<div class="formula">
  f<sub>PWM</sub> = 170,000,000 / 8500 = 20,000 Hz
</div>

Cette configuration donne donc une fréquence de PWM de **20,0000 Hz**, ce qui est exactement la fréquence cible.