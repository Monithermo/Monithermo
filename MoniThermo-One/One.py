#!/usr/bin/env python
# -*- coding:Utf-8 -*-

import subprocess, time, os

########################################################################################################################################################################
##
##                      Role : Programme permettant de traiter le fichier " stock " cree par le micro-controleur.
##			       Le fichier stock est constitue de la maniere suivante : 
##					- nom du capteur,
##					- timestamp,
##					- temperature
##		
##                      Auteur : MoniThermo
##                      Creation : 05/11/2014
##                      Modification :
##                      License: GNU GPL v2 (cf : " license.txt " )
##
##
##			Liste des definitions : -> def erreur (msg)
##			  			-> def log (lmsg)
##
########################################################################################################################################################################

nb = 0


			  ###################
			  ### definitions ###
			  ###################

                                     
			######################
			### Log et Erreurs ###
			######################

###################################################################################
##										 ##
##	Role : Stocker l'erreur dans un fichier dedie				 ##
##										 ##
##	Entree : msg -> message d'erreur a stocker				 ##
##	Sortie : /								 ##
##										 ##
###################################################################################
def erreur (msg) :
        time_erreur = time.strftime("%d-%m-%Y : %H:%M:%S : ", time.gmtime())
        erreur = open("/MoniThermo/suivi/erreur.txt", "append")
        erreur.write(time_erreur + """erreur prg "monithermo.py" : """ + msg + "\n")
        erreur.close()


###################################################################################
##										 ##
##	Role : Logger dans un fichier le deroulement du programme		 ##
##										 ##
##	Entree : lmsg -> message de log	 					 ##
##	Sortie : /								 ##
##										 ##
###################################################################################
def log (lmsg):
        time_log = time.strftime("%d-%m-%Y : %H:%M:%S : ", time.gmtime())
        erreur = open("/MoniThermo/suivi/log.txt", "append")
        erreur.write(time_log + """log prg "monithermo.py" : """ + lmsg + "\n")
        erreur.close()

				###########################
				### fin des definitions ###
				###########################

						
###################################################################################
##										 ##
##				Programme principal				 ##
##										 ##
##		Algorithme : -> Recuperation des donnees du capteur		 ##
##			     -> Creation fichier serveur			 ##
##			     -> Stockage des donnees				 ##
##										 ##
###################################################################################
try :
	# Ouverture du fichier qui contient les informations donnees par le micro-controleur
	log("ouverture fichier stock")
        
        stock = open("/MoniThermo/tmp/stock.txt", "read")
        data = stock.readline()
        stock.close()
	
	log("data : " + data)
	
        # " capteur,timestamp,temperature "
        temp = data.split(',')
        print temp

	# Enregistrement des donees
	log("enregistrement donnees...")
	
	log("fichier serveur")
	# enregistrement dans fichier serveur
	try :
		# Eviter que le fichier serveur soit trop lourd
		valid = False
		while valid != True : 
			if os.path.exists("/MoniThermo/tmp/fichier_serveur" + str(nb)) == True :
				lignes = subprocess.check_output("wc -l /MoniThermo/tmp/fichier_serveur" + str(nb), shell = True)
				lignes = lignes.split(" ")
				print lignes
				if eval(lignes[0]) >= 500 :
					nb = nb + 1
					valid = False
				else : 
					valid = True
			else : 
				valid = True
		serveur_T = open("/MoniThermo/tmp/fichier_serveur" + str(nb), "append")
		serveur_T.write(temp[1] + ',' + temp[0] + ',,' + realTemp +  '\n')
		serveur_T.close()
	
	except Exception, e :
		print "Erreur : %s", e
		erreur("fichier serveur : " + str(e))
	
	log("fichier suivi")
	# enregistrement dans fichier suivi
       	file_suivi = open("/MoniThermo/suivi/" + temp[0] + ".txt", "append")
       	file_suivi.write("capteur : " + temp[0] + ", heure UTC : " + time.strftime("%d-%m-%y : %H:%M:%S", time.gmtime(int(temp[1]))) + ", temperature : " + realTemp + '\n')
       	file_suivi.close()
	
	# Fin du programme principal
	log("fin monithermo\n------------------------------------------------------------------------------------")

except Exception, e :
	print "erreur prg"
	print "Erreur : %s", e
	erreur(str(e))
