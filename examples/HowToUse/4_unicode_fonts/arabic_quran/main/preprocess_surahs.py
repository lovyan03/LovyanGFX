import os
import json

def create_surah_files(json_file):
    # Load the Quran JSON data
    with open(json_file, 'r', encoding='utf-8') as file:
        quran_data = json.load(file)

    # Create a directory named "surahs" if it doesn't exist
    surah_dir = "surahs"
    if not os.path.exists(surah_dir):
        os.makedirs(surah_dir)
    else:
        print('ERROR: f{surah_dir} folder already exists. exsiting in fear of overwriting.')
        return

    # Dictionary to hold surah data grouped by surah number
    surahs = {}

    # Organize ayahs by surah
    for entry in quran_data:
        sura_no = entry["sura_no"]
        aya_text = entry["aya_text"].replace('\u200f', '')  # Remove 0x200f characters

        if sura_no not in surahs:
            surahs[sura_no] = []
        surahs[sura_no].append(aya_text)

    # Write each surah to its own text file
    for sura_no, ayahs in surahs.items():
        file_path = os.path.join(surah_dir, f"{sura_no}.txt")
        with open(file_path, 'w') as surah_file:
            # Write the number of ayahs at the top of the file
            surah_file.write(f"{len(ayahs)}\n")
            # Write each aya_text line by line
            for aya in ayahs:
                surah_file.write(f"{aya}\n")

    print("Surah files created successfully in the 'surahs' folder.")

# Example usage:
create_surah_files("hafs.json")

print("Surahs folder created in workind directory. please move it into the flash folder")

