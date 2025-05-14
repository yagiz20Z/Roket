import os
import datetime

# Çalışılan dizin
target_dir = os.getcwd()

# Header'ı oluştur
def create_header(file_path):
    try:
        created_ts = os.path.getctime(file_path)
        dateData = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        created_date = datetime.datetime.fromtimestamp(created_ts).strftime("%Y-%m-%d %H:%M:%S")
    except Exception:
        created_date = "Tarih alınamadı"

    # ASCII Art içeren header (raw f-string: rf""")
    header = rf"""/*
*
*        ^
*       /^\\
*      /___\\         YILDIZ ROKET TAKIMI
*     |=   =|        
*     |     |        Olusturulma Tarihi: {created_date}
*     |     |        Tarih: {dateData}
*     |     |
*    /|##!##|\\
*   / |##!##| \\
*  /  |##!##|  \\
* |  / ^ | ^ \  |                                       YILDIZDAN
* | /  ( | )  \ |
* |/   ( | )   \|                                                                YILDIZLARA
*     ((   ))                                                                       
*    ((  :  ))
*    ((  :  ))
*     ((   ))
*      (( ))                     
*       ( )
*        .
*        .
*        .
*/
"""
    return header

# Dosyaları gez ve header ekle
for root, dirs, files in os.walk(target_dir):
    for file in files:
        if file.endswith(('.c', '.h')):
            file_path = os.path.join(root, file)
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()

            if "YILDIZ ROKET TAKIMI" not in content:
                header = create_header(file_path)
                with open(file_path, 'w', encoding='utf-8') as f:
                    f.write(header + '\n' + content)
                print(f"✅ Header eklendi: {file}")
            else:
                print(f"⏭️ Zaten header var: {file}")

print("🎉 Tüm işlem tamamlandı.")
