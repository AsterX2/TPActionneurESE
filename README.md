# TPActionneurESE

# Compte Rendu : Contrôle de MCC avec la Carte Nucleo-G474R



## 6. TP n°1 - Commande MCC basique

## 6.1 Génération de 4 PWM

On génère quatre PWM sur les bras de pont U et V afin de contrôler le hacheur à partir du timer 1 qui est déjà attrribué sur les pins.

D'après le cahier des charges (cdc), la fréquence de la PWM est de 20 kHz ce qui ne devrait pas être audible. Les temps mort minimum sont détaillés dans la suite. Et la résolution est de 10 bits. Pour les tests, on fixe le rapport cyclique à 60%.

Une fois les PWM générées, on les affichent sur un oscilloscope à l'onde d'une sonde numérique pour visualiser les PWM configurées :

| marron | orange | noir | rouge |
| ------ | ------ | ---- | ----- |
| pb13   | pb14   | pa8  | pa9   |





<img src="./WhatsApp%20Image%202024-11-06%20at%2019.20.49-1730977352039-2.jpeg" alt="WhatsApp Image 2024-11-06 at 19.20.49" style="zoom: 25%;" />

**Paramètres configurés :**

- **Fréquence** : 20 kHz
- **Résolution** : 10 bits
- **Temps mort** : 200 ns, validé par la datasheet des transistors (Rise Time + Turn on delay Time=90 ns).

**Étapes effectuées :**
- Configuration des pins pour les canaux PWM de TIM1.
- Paramétrage des sorties complémentaires et des temps morts dans CubeIDE.

f_PWM =fhorloge_timer/(PSC+1)×(ARR+1)

f_PWM = 170,000,000 / (2 * 2 * (2124 + 1))

f_PWM = 170,000,000 / 8500 = 20,000 Hz



**Résultats :**
- Signaux PWM observés et validés à l'oscilloscope

![image-20241107120818936](./image-20241107120818936.png)

---

<img src="./image-20241107183303442.png" alt="image-20241107183303442" style="zoom: 50%;" />



Deux registres sont utilisés pour configurer le signal PWM en mode center-aligned :

- ARR (Auto-Reload Register): Détermine la période du compteur, donc la fréquence du PWM.
- CCR (Capture Compare Register): Définit le moment où le signal PWM change d'état, contrôlant ainsi le rapport cyclique.

## Fonctionnement du Mode Center-Aligned

En mode center-aligned, le compteur du timer effectue un comptage ascendant puis descendant:

- Phase montante: Le compteur compte de 0 jusqu'à la valeur ARR.
- Phase descendante : Le compteur compte de ARR jusqu'à 0.

Ce mode crée une forme d'onde triangulaire pour le compteur, par opposition au mode edge-aligned où le compteur ne fait qu'incrémenter.

## Rôle de l'ARR

- Définit la période du PWM : La période totale du signal PWM est déterminée par le temps nécessaire au compteur pour effectuer une **montée** et une **descente** complète.

- Formule de la fréquence PWM en mode center-aligned :

  ​											f_PWM =fhorloge_timer/2*(PSC+1)×(ARR+1)

  - Le facteur 2 est dû au fait que le compteur monte et descend, donc le temps total est doublé par rapport au mode edge-aligned.

## Rôle du CCR

- Contrôle le rapport cyclique : La valeur du CCR détermine les points où le signal PWM change d'état pendant le cycle de comptage.
- Commutation du signal :
  - En montée : Lorsque le compteur atteint la valeur CCR, le signal PWM change d'état (par exemple, passe à l'état haut).
  - En descente : Lorsque le compteur redescend et atteint à nouveau CCR, le signal PWM revient à son état initial (par exemple, passe à l'état bas).

## Calcul du Rapport Cyclique 

Le rapport cyclique est déterminé par la valeur du CCR par rapport à ARR.

- **Formule du rapport cyclique** :

  rapport cyclique=ARR/CCR

- **Interprétation** : Le rapport cyclique est le ratio entre la durée de conduction de l'interrupteur et la période de fonctionnement (T).

## Avantages du Mode Center-Aligned

- Réduction des Harmoniques : Le mode center-aligned génère moins d'harmoniques de rang impair, ce qui réduit le bruit électromagnétique.
- Symétrie du Signal : Les fronts montants et descendants sont centrés, ce qui est utile pour certaines applications nécessitant une symétrie parfaite.





On implémente aussi des temps morts de 200 ns estimé à l'aide du fall time et rise time, plus, leurs temps annexes dans la doc des transistors:

![WhatsApp Image 2024-11-25 at 22.15.03](./repoimg/WhatsApp%20Image%202024-11-25%20at%2022.15.03.jpeg)











## 6.2 Commande de Vitesse via UART

Pour contrôler la vitesse du moteur, nous allons ajouté une commande `speed XXXX` qui sera envoyé par l'UART.

**Commande implémentée :**

- **Format** : `speed XXXX` pour ajuster la vitesse du moteur.

**Étapes effectuées :**

- Configuration de l’UART pour communication série avec terminal.
- Détection et traitement de la commande `speed` via putty.
- Application de la vitesse demandée par ajustement du rapport cyclique PWM.

Après avoir lu le token `"XXXX"` de `speed XXXX`, on vérifie sa conformité avec l'intervalle de valeurs extremum du cdc, plus une marge de sécurité.

On peut ensuite modifier le rapport cyclique de nos 4 canaux PWM via les fonctions `__HAL_TIM_SET_Compare()`. Il suffit de modifier les channels 1 et 2 car leurs complémentaires ont le même rapport cyclique.

- La commande de vitesse fonctionne, avec validation des valeurs limites (0 à 2174).

---

## 6.3 Premiers Tests de Contrôle du Moteur

**Conditions de test :**

- Rapports cycliques : 50 % et 70 %

**Observations :**

- Comme attendu les phases U et V se compensent à 50% du fait du mode center aligned.
- Courants d'appel limités, améliorant la sécurité des transistors.
- On remarque que les à-coups sont importants et pourrait nuire au moteur. Il convient donc de faire monter le rapport cyclique sur une rampe progressive.

**Vue de oscilloscope à un rapport cyclique de 40% et 60% :**

![tek00000](./repoimg/tek00000.png)

![tek00001](./repoimg/tek00001.png)



## 7. TP n°2 - Commande en boucle ouverte, mesure de vitesse et de courant

### 7.1. Commande de la vitesse

On a implémenter des fonctions pour commander le moteur tels que, `start_PWM(TIM_HandleTypeDef htim,uint32_t channel)` et `stop_PWM(TIM_HandleTypeDef htim,uint32_t channel)` afin de démarrer la génération des PWM et arrêter le rapport cyclique. 



Pour générer les PWM on utilise les fonctions `HAL_TIM_PWM_Start` et `HAL_TIMEx_PWMN_Start` . Pour la fixation des rapports cyliques à 50 %, voir les commentaires du code contenu dans le fichier `cmd_speed.c`. 

De même, nous avons codé la fonction `stop_PWM(TIM_HandleTypeDef htim,uint32_t channel)`  à l'aide `HAL_TIM_PWM_Stop()` 



### 7.2. Mesure du courant

A partir de la documentation (schéma KiCad) : 

- Définir quel(s) courant(s) vous devez mesurer,

  Nous devons mesurer U_Imes et V_Imes. La mesure de courant est effectué par GO 10-SME/SP3 puis est transmise aux pins PA1 et PB1 de la nucleo.

- Définir les fonctions de transfert des capteurs de mesure de courant (lecture datasheet),

On considère le gain et l'offset du capteur, sensitivity à 50 mV/A et 1,65V pour la reference de tension

![image-20241113165546988](./repoimg/WhatsApp%20Image%202024-11-25%20at%2014.33.58.jpeg)

Soit, 

​									Vout= 1,65 + 0.05 * Imes

Nous allons maintenant pouvoir configurer l'ADC en fonction de ceci.

- Déterminer les pin du stm32 utilisés pour faire ces mesures de courant,

PA1 et PB1, d'aprés le shematic et la doc.

- Établir une première mesure de courant avec les ADC en Pooling. Faites des tests à vitesse nulle, non nulle, et en charge (rajouter un couple résistif en consommant du courant sur la machine synchrone couplée à la MCC).



#### Configuration de l'ADC en mode Pooling 

Lorsque l'utilisateur rentre `current`, une mesure du courrant est réalisée. C'est pourquoi les morceaux de code suivants doivent être placés dans la section de code du shell loop.

On va prélever la mesure de l'ADC. Pour cela il faut activer l'ADC avec `HAL_ADC_Start(&hadc1)` 
 et lancer la conversion par pooling avec 

```C
HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
uint16_t adcBuff;
adcBuff=HAL_ADC_GetValue(&hadc1);
```

On récupère la variable de l'adc avec `HAL_ADC_GetValue(&hadc1).`  

Ce  moyen d'obtenir le courant est par polling, sur demande, car c'est lorsque l'utilisateur appelle avec `current` que le processeur en mode bloquant va chercher la valeur de l'adc.



Le problème avec cette méthode c'est que l'ADC occupe le processeur, ce qui est incompatible avec un asservissement.

C'est pourquoi nous allons implémenter la méthode par DMA 



#### Configuration de l'ADC en mode DMA

On va mettre en place d'une la chaîne d'acquisition Timer/ADC/DMA.

On commence par démarrer l'ADC en mode DMA dans le main.c avec HAL_ADC_Start_DMA(&hadc1).

On utilise le Timer 1 qui génère les pwm pour déclencher un Update Event. Le paramétrage se fait ici :  




![image-20241125183823804](./repoimg/image-20241125183823804.png)

​     

Dans l'ADC, on paramètre le lancement de la conversion de l'ADCsur un événement externe en l'occurence sur le TIM1 (en Trigger Output Event).
![image-20241125234352010](./repoimg/image-20241125234352010.png)

Dans NVIC TIM1, on coche Update interrupt.

![image-20241120173111108](./repoimg/image-20241120173111108.png)

​										Valeurs de l'adc avec la méthode du DMA

![image-20241120180534746](./repoimg/image-20241120180534746.png)





### 7.3. Mesure de vitesse

### **Compréhension du Capteur de Vitesse**

Le capteur de vitesse qui est un encodeur incrémental, génère des impulsions électriques proportionnelles à la rotation de l'arbre du moteur. 

#### **Fonction de Transfert du Capteur :**

- **Nombre d'impulsions par tour (N) :** Indique le nombre d'impulsions générées pour une rotation complète
- **Fréquence des impulsions (f) :**Hz
- **Vitesse de rotation (ω) :** En tours par minute (RPM)

**Formule :**

![image-20241125152046559](./repoimg/image-20241125152046559.png)



## 8. TP n°3 Asservissement

Nous n'avons pas pu implémenter cette partie mais nous expliquons ici les étapes nécessaires d'un point de vue théorique.

## Asservissement en Vitesse :

L'asservissement en vitesse consiste à contrôler la vitesse du moteur de manière à ce qu'elle suive une consigne donnée, malgré les perturbations externes (charges, variations de tension, ...). Pour cela, nous avons besoin d'une boucle de rétroaction où la vitesse réelle est mesurée et comparée à la vitesse souhaitée.

### Étapes :

1. **Mesure de la Vitesse**:

   - Utiliser le codeur incrémental du moteur pour obtenir la vitesse réelle.
   - Convertir les impulsions du codeur en une valeur de vitesse (rad/s).

2. **Calcul de l'Erreur**:

   - Calculer l'erreur de vitesse: 

     ​		Erreur=Consigne de vitesse−Vitesse mesurée

3. **Régulateur PI**:

   - Implémenter un régulateur proportionnel-intégral (PI) pour corriger l'erreur.

4. **Application de la Commande**:

   - Ajuster le rapport cyclique de la PWM en fonction de la sortie du régulateur.
   - Les limites sont fixées dans le codes à 10% et 90% du rapport cyclique par sécurité

5. **Boucle de Contrôle**:

   - On doit exécuter ces calculs à une fréquence régulière pour que l'asservissement soit efficace

     

## Asservissement en Courant

- **Objectif**: Contrôler le courant consommé par le moteur pour protéger le système et améliorer la performance.

### Étapes Clés:

1. **Mesure du Courant**:
   - Acquérir ces mesures via l'ADC du microcontrôleur.
2. **Calcul de l'Erreur de Courant**:
   - Calculer l'erreur de courant:
     Erreur de courant=Consigne de courant−Courant mesure
3. **Régulateur de Courant**:
   - Implémenter un régulateur PI pour minimiser l'erreur de courant.
   - Ajuster suffisamment rapidement la commande pour répondre aux variations de charge.
4. **Boucle Rapide**:
   - Exécuter cette boucle à une fréquence plus élevée que celle de la vitesse.

## Intégration finale des deux asservissements

- Structure en Cascade:
  - **Boucle externe**: Asservissement en vitesse.
  - **Boucle interne**: Asservissement en courant.
- Avantages:
  - Réponse rapide aux perturbations.
  - Meilleure stabilité et précision du système. Nous aurions pu ensuite les mesurée depuis notre code.







## Auteurs

Vincent Lakhmeche	

Karim Jerjoub















