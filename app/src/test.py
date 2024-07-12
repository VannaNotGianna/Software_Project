import os
import re
from tqdm import tqdm
import argparse

def test(nbr):
    folder_name = os.path.basename(os.path.normpath(os.path.join(os.getcwd(), "coiu")))
    output_file = f"{folder_name}.txt"
    with open(output_file, "w") as file:
        file.write(nbr*2)

    print(f"Fichier {output_file} créé avec succès.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Exécute la fonction test avec un nombre donné.")
    parser.add_argument("nbr", type=int, help="Le nombre à doubler et écrire dans le fichier.")
    args = parser.parse_args()

    test(args.nbr)