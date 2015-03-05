#!/usr/bin/env python
# -*- coding:Utf-8 -*-
import subprocess, time


########################################################################################################################################################################
##
##			Fonction : Programme permettant de poster la température sur le serveur
##			Auteur : Monithermo
##			Creation : 27/02/2014
##			Modification :  -> 24/10/2014 : ajout sous-fonction
##			License: GNU GPL v2 (voir license.txt)
##
########################################################################################################################################################################

URL = "MonNomDeDomaine.com/PageDuPost"

# Définitions du sous-programme permettant de logger les erreurs
def erreur (msg): 
	time_erreur = time.strftime("%d-%m-%Y : %H:%M:%S : ")
	erreur = open("/erreur.txt", "append")
	erreur.write(time_erreur + """erreur prg "post_temp.py" : """ + msg + "\n")
	erreur.close()
	
try :
        # On s'assure que le Yún est bien connecté à internet
        rep = subprocess.call("ping -c4 monithermo.com", shell = True)
		
	if rep == 0 : 
		print "connexion : ok"
		subprocess.call("blink-stop", shell = True)
		
                # Commande curl pour envoyer un fichier vers un serveur
                rep = subprocess.check_output(["curl", "--form", "f=@/fichier_serveur.txt", URL])
        	print rep
                # Si le fichier est reçu => effacer le fichier serveur
        	if rep == "OK\n" :
                	subprocess.call("rm /fichier_serveur.txt", shell = True)
                        print "file remove"
        	else : 
        		print "error fichier"
                	erreur("fichier_serveur.txt, non effacer")

        else :
                # Clignotement de la DEL bleu (wlan)
		subprocess.call("blink-start 500", shell = True)
		print "erreur connexion"
		erreur("ping")
		
except Exception, e :
	print "erreur programme"
	erreur("erreur : ", str(e))

