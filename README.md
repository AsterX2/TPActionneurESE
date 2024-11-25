# TPActionneurESE

- # Compte Rendu : Contrôle de MCC avec la Carte Nucleo-G474RE

  ## Sommaire
  - [Introduction](#introduction)
  - [Objectifs](#objectifs)
  - [6.1 Génération de 4 PWM](#61-génération-de-4-pwm)
  - [6.2 Commande de Vitesse via UART](#62-commande-de-vitesse-via-uart)
  - [6.3 Premiers Tests de Contrôle du Moteur](#63-premiers-tests-de-contrôle-du-moteur)
  - [Conclusion](#conclusion)

  ---

  ## Introduction

  Ce document rend compte de la configuration et des tests d'un système de contrôle moteur en boucle ouverte utilisant une carte Nucleo-G474RE.

  ## Objectifs

  1. Générer des PWM pour le contrôle moteur.
  2. Implémenter une interface de commande de vitesse via UART.
  3. Valider le fonctionnement pratique du système.

  ---

  ## 6.1 Génération de 4 PWM

  Cablage de la sonde numérique :
  
  marron : pb13
  
  orange : pb14
  
  noir : pa8
  
  rouge:  pa9
  
  
  
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
  - Signaux PWM observés et validés à l'oscilloscope (voir illustration ci-dessous).
  
  **Illustrations :**
  
  <img src="./WhatsApp%20Image%202024-11-06%20at%2019.20.49-1730977352039-2.jpeg" alt="WhatsApp Image 2024-11-06 at 19.20.49" style="zoom: 25%;" />
  
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
  
  - Définit la période du PWM : La période totale du signal PWM est déterminée par le temps nécessaire au compteur pour effectuer une montée et une descente complète.
  
  - Formule de la fréquence PWM en mode center-aligned :
  
    ​											f_PWM =fhorloge_timer/2*(PSC+1)×(ARR+1)
  
    - Le facteur 2 est dû au fait que le compteur monte et descend, donc le temps total est doublé par rapport au mode edge-aligned.
  
  ## Rôle du CCR
  
  - Contrôle le rapport cyclique : La valeur du CCR détermine les points où le signal PWM change d'état pendant le cycle de comptage.
  - Commutation du signal :
    - En montée : Lorsque le compteur atteint la valeur CCR, le signal PWM change d'état (par exemple, passe à haut).
    - En descente : Lorsque le compteur redescend et atteint à nouveau CCR, le signal PWM revient à son état initial (par exemple, passe à bas).
  
  ## Calcul du Rapport Cyclique 
  
  Le rapport cyclique est déterminé par la valeur du CCR par rapport à ARR.
  
  - **Formule du rapport cyclique** :
  
    rapport cyclique=ARR/CCR
  
  - **Interprétation** : Le rapport cyclique est le ratio entre le temps pendant lequel le signal est actif (état haut) et la période totale.
  
  ## Avantages du Mode Center-Aligned
  
  - Réduction des Harmoniques : Le mode center-aligned génère moins d'harmoniques de rang impair, ce qui réduit le bruit électromagnétique.
  - Symétrie du Signal : Les fronts montants et descendants sont centrés, ce qui est utile pour certaines applications nécessitant une symétrie parfaite.
  
  
  
  
  
  On implémente aussi des temps morts de 100 ns estimé à l'aide du fall time et rise time plus leurs temps annexes dans la doc des transistors:
  
  
  
  
  
  ## 6.2 Commande de Vitesse via UART
  
  **Commande implémentée :**
  - **Format** : `speed XXXX` pour ajuster la vitesse du moteur.
  
  **Étapes effectuées :**
  
  - Configuration de l’UART pour communication série avec terminal.
  - Détection et traitement de la commande `speed` via putty.
  - Application de la vitesse demandée par ajustement du rapport cyclique PWM.
  
  Après avoir lu le token "XXXX" de speed XXXX, on vérifie la conformité avec l'intervalle de valeurs extremum du cdc plus une marge de sécurité.
  
  On peut ensuite modifier le rapport cyclique de nos 4 canaux PWM via les fonctions HAL_TIM_SET_Compare(). Il suffit de modifier les channels 1 et 2 car leurs complémentaires ont le même rapport cyclique.
  
  - La commande de vitesse fonctionne, avec validation des valeurs limites (0 à 2174).
  
  ---
  
  ## 6.3 Premiers Tests de Contrôle du Moteur
  
  **Conditions de test :**
  - Rapports cycliques : 50 % et 70 %
  
  **Observations :**
  - Comme attendu les phases U et V se compensent à 50% du fait du mode center alined.
  - Courants d'appel limités, améliorant la sécurité des transistors.
  - 
  
  **Vue de oscilloscope à un rapport cyclique de 40% et 60% :**
  
  ![tek00000](./repoimg/tek00000.png)
  
  ![tek00001](./repoimg/tek00001.png)
  
  
  
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
  
  - Etablir une première mesure de courant avec les ADC en Pooling. Faites des  tests à vitesse nulle, non nulle, et en charge (rajouter un couple resistif en consommant du courant sur la machine synchrone couplée à la  MCC).
  
  
  
  ![image-20241120173111108](./repoimg/image-20241120173111108.png)
  
  
  
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

   - Utiliser le codeur du moteur pour obtenir la vitesse réelle.
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























