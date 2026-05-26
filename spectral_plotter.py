import numpy as np
import matplotlib.pyplot as plt
import serial
import os
import struct
import pandas as pd
from datetime import datetime
from tkinter import Tk, filedialog, ttk, messagebox, StringVar, Label, Button
from serial.tools import list_ports

def receive_and_save_data(ser, bin_filename, packet_size=4096, max_packets=2048*64):
    """Receives packets via serial COM VCP and saves them to a binary file."""
    with open(bin_filename, 'wb') as f:
        for packet_count in range(max_packets):
            data = ser.read(packet_size)
            if not data:
                continue
            if data == b'T':  # Terminator character compatible with mainboard VCP
                print("Received Terminator Character.")
                break
            f.write(data)
            print(f"📥 Received NAND page packet {packet_count+1}")
    ser.close()
    print("📴 Serial COM Port Closed.")

def process_bin_file(bin_filename, csv_filename=None):
    """Parses binary NAND data pages and plots spectral intensities and flicker."""
    hh_list, mm_list, ss_list = [], [], []
    luce_list = []
    blue_list, deep_blue_list, clear_list = [], [], []

    with open(bin_filename, "rb") as f:
        while True:
            pagina = f.read(4096)
            if len(pagina) < 4096:
                break
            
            # Consider only valid sample bytes (292 samples * 14 bytes = 4088 bytes)
            valid_bytes = pagina[:4088]
            
            # Parse each 14-byte subpacket
            for i in range(0, len(valid_bytes), 14):
                subpkt = valid_bytes[i:i+14]
                if len(subpkt) < 14:
                    continue
                
                # Unpack 7 uint16_t little-endian values:
                # hh, mm, ss, luce_artificiale, blue, deep_blue, clear
                hh, mm, ss, luce_art, blue, deep_blue, clear = struct.unpack("<7H", subpkt)
                
                # Check for sentinel / padding values (0xFFFF) used at end-of-data
                if hh == 65535 or mm == 65535 or ss == 65535:
                    continue  # Ignore padding
                    
                hh_list.append(hh)
                mm_list.append(mm)
                ss_list.append(ss)
                luce_list.append(luce_art)
                blue_list.append(blue)
                deep_blue_list.append(deep_blue)
                clear_list.append(clear)

    if not hh_list:
        print("⚠️ No valid spectral samples found in binary file.")
        return

    # Create formatted timestamp string
    time_strings = [f"{h:02d}:{m:02d}:{s:02d}" for h, m, s in zip(hh_list, mm_list, ss_list)]

    # Create pandas DataFrame
    df = pd.DataFrame({
        "Time": time_strings,
        "hh": hh_list,
        "mm": mm_list,
        "ss": ss_list,
        "luce_artificiale": luce_list,
        "blue": blue_list,
        "deep_blue": deep_blue_list,
        "clear": clear_list
    })

    # Plot spectral intensities
    plt.figure(figsize=(12, 8))
    
    # Subplot 1: AS7341 Spectral Channels
    plt.subplot(2, 1, 1)
    plt.plot(df.index, df["deep_blue"], label="Deep Blue (F1 - 415nm)", color="#00008B", linewidth=1.5)
    plt.plot(df.index, df["blue"], label="Blue (F2 - 445nm)", color="#00BFFF", linewidth=1.5)
    plt.plot(df.index, df["clear"], label="Clear (CH4/F5 - 555nm)", color="#FF8C00", linewidth=1.5)
    plt.title("AS7341 Spectral Channels Physical Intensity", fontsize=12, fontweight='bold', pad=10)
    plt.ylabel("Intensity (Scaled Physical Value)", fontsize=10)
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.legend(loc="upper right")
    
    # Subplot 2: Artificial Light Detection (Flicker)
    plt.subplot(2, 1, 2)
    plt.step(df.index, df["luce_artificiale"], label="Artificial Light Detected", color="#FF0000", where="post", linewidth=1.5)
    plt.title("Flicker Detection Status (100Hz / 120Hz)", fontsize=12, fontweight='bold', pad=10)
    plt.ylabel("Status (0=None, 1=Detected)", fontsize=10)
    plt.xlabel("Sample Index", fontsize=10)
    plt.ylim(-0.2, 1.2)
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.legend(loc="upper right")
    
    plt.tight_layout()
    plt.show()

    # Save to CSV
    if csv_filename:
        df.to_csv(csv_filename, index=False)
        print(f"📄 Data saved in CSV: {csv_filename}")

def gui_select_com_and_folder():
    """Opens a small GUI for selecting the COM port and folder."""
    root = Tk()
    root.title("Spectral Data Logger - Configuration")
    root.geometry("400x400")
    root.resizable(False, False)

    # Styling
    style = ttk.Style()
    style.theme_use("clam")

    Label(root, text="🔌 Select the COM Port:", font=("Segoe UI", 10, "bold")).pack(pady=10)
    
    com_var = StringVar()
    ports = [p.device for p in list_ports.comports()]

    if not ports:
        ports = ["No COM Port found"]
    com_box = ttk.Combobox(root, textvariable=com_var, values=ports, state="readonly", width=30)
    com_box.pack(pady=5)
    com_box.current(0)

    def browse_folder():
        folder = filedialog.askdirectory(title="📂 Select the folder to save data")
        if folder:
            folder_var.set(folder)

    folder_var = StringVar()
    Label(root, text="📁 Destination Folder:", font=("Segoe UI", 10, "bold")).pack(pady=15)
    Button(root, text="Browse Folder...", command=browse_folder, font=("Segoe UI", 9)).pack()
    Label(root, textvariable=folder_var, fg="blue", font=("Segoe UI", 9, "italic"), wraplength=350).pack(pady=10)

    def confirm():
        if not folder_var.get() or "No COM Port" in com_var.get():
            messagebox.showerror("Error", "Select a valid COM Port and a destination folder.")
            return
        root.destroy()

    Button(root, text="✅ Start Download", command=confirm, bg="#4CAF50", fg="white", font=("Segoe UI", 10, "bold"), width=20, height=2).pack(pady=20)
    root.mainloop()

    return com_var.get(), folder_var.get()

# ==============================
# Main
# ==============================

def main():
    com_port, save_path = gui_select_com_and_folder()
    if not com_port or not save_path:
        print("❌ Application Stopped.")
        return

    base_filename = datetime.now().strftime("SpectralData_%Y%m%d_%H%M%S")
    bin_filename = os.path.join(save_path, f"{base_filename}.bin")
    csv_filename = os.path.join(save_path, f"{base_filename}_spectral.csv")

    BAUD_RATE = 250000

    try:
        ser = serial.Serial(com_port, BAUD_RATE, timeout=10)
        print(f"🔌 Connected to {com_port}")
        receive_and_save_data(ser, bin_filename)
    except serial.SerialException as e:
        print(f"⚠️ Serial Error: {e}")
        return

    process_bin_file(bin_filename, csv_filename)

if __name__ == "__main__":
    main()