# ============================================================
# 无线蓝牙翻谱器上位机，配套V2.2
# by motozilog
# 2026-07-28
# ============================================================

import tkinter as tk
from tkinter import messagebox, ttk
import hid
import time

# ============================================================
# HID 键盘键码映射表 (Usage ID -> 显示名称)
# ============================================================
KEY_MAP = {
    # 字母键 (0x04 - 0x1D)
    0x04: "A", 0x05: "B", 0x06: "C", 0x07: "D", 0x08: "E",
    0x09: "F", 0x0A: "G", 0x0B: "H", 0x0C: "I", 0x0D: "J",
    0x0E: "K", 0x0F: "L", 0x10: "M", 0x11: "N", 0x12: "O",
    0x13: "P", 0x14: "Q", 0x15: "R", 0x16: "S", 0x17: "T",
    0x18: "U", 0x19: "V", 0x1A: "W", 0x1B: "X", 0x1C: "Y",
    0x1D: "Z",
    
    # 数字键 (0x1E - 0x27)
    0x1E: "1", 0x1F: "2", 0x20: "3", 0x21: "4", 0x22: "5",
    0x23: "6", 0x24: "7", 0x25: "8", 0x26: "9", 0x27: "0",
    
    # 功能键 (0x28 - 0x3F)
    0x28: "Enter", 0x29: "Esc", 0x2A: "Backspace",
    0x2B: "Tab", 0x2C: "Space", 0x2D: "-_", 0x2E: "=+",
    0x2F: "[{", 0x30: "]}", 0x31: "\\|", 0x32: ";:",
    0x33: "'\"", 0x34: "`~", 0x35: ",<", 0x36: ".>",
    0x37: "/?", 0x38: "CapsLock",
    
    # F1 - F12 (0x3A - 0x45)
    0x3A: "F1", 0x3B: "F2", 0x3C: "F3", 0x3D: "F4",
    0x3E: "F5", 0x3F: "F6", 0x40: "F7", 0x41: "F8",
    0x42: "F9", 0x43: "F10", 0x44: "F11", 0x45: "F12",
    
    # 控制键 (0x46 - 0x65)
    0x46: "PrintScreen", 0x47: "ScrollLock", 0x48: "Pause",
    0x49: "Insert", 0x4A: "Home", 0x4B: "PageUp",
    0x4C: "Delete", 0x4D: "End", 0x4E: "PageDown",
    0x4F: "→", 0x50: "←", 0x51: "↓", 0x52: "↑",
    
    # NumLock 和数字键盘 (0x53 - 0x65)
    0x53: "NumLock", 0x54: "KP/", 0x55: "KP*",
    0x56: "KP-", 0x57: "KP+", 0x58: "KPEnter",
    0x59: "KP1", 0x5A: "KP2", 0x5B: "KP3",
    0x5C: "KP4", 0x5D: "KP5", 0x5E: "KP6",
    0x5F: "KP7", 0x60: "KP8", 0x61: "KP9",
    0x62: "KP0", 0x63: "KP.", 0x64: "KP=",
    
    # 修饰键 (0xE0 - 0xE7)
    0xE0: "Ctrl", 0xE1: "Shift", 0xE2: "Alt",
    0xE3: "Win", 0xE4: "Ctrl", 0xE5: "Shift",
    0xE6: "Alt", 0xE7: "Win",
    
    # 多媒体键 (0xE9 - 0xEF)
    0xE9: "音量+", 0xEA: "音量-", 0xEB: "静音",
    0xEC: "播放", 0xED: "停止", 0xEE: "上一曲",
    0xEF: "下一曲", 0xF0: "暂停", 0xF1: "主页",
    0xF2: "邮件", 0xF3: "搜索", 0xF4: "收藏",
    0xF5: "刷新", 0xF6: "前进", 0xF7: "后退",
    
    # 额外功能键
    0x65: "应用程序键", 0x66: "电源", 0x67: "KP=",
    0x68: "F13", 0x69: "F14", 0x6A: "F15",
    0x6B: "F16", 0x6C: "F17", 0x6D: "F18",
    0x6E: "F19", 0x6F: "F20", 0x70: "F21",
    0x71: "F22", 0x72: "F23", 0x73: "F24",
    0x74: "执行", 0x75: "帮助", 0x76: "菜单",
    0x77: "选择", 0x78: "停止", 0x79: "再次",
    0x7A: "撤销", 0x7B: "剪切", 0x7C: "复制",
    0x7D: "粘贴", 0x7E: "查找", 0x7F: "静音",
    0x80: "音量+", 0x81: "音量-",
    
    # 消费类控制 (多媒体)
    0xB5: "下一曲", 0xB6: "上一曲", 0xB7: "停止",
    0xCD: "播放/暂停", 0xE2: "静音",
}

# 键码列表 (按值排序)
KEY_LIST = sorted(KEY_MAP.items(), key=lambda x: x[0])

# 显示名称列表 (用于 ComboBox 下拉)
KEY_NAMES = [f"{name} (0x{code:02X})" for code, name in KEY_LIST]

# 显示名称 -> 键码值 的反向映射
NAME_TO_CODE = {f"{name} (0x{code:02X})": code for code, name in KEY_MAP.items()}


def calc_checksum(data):
    """计算 16 字节数据的校验和 (累加)"""
    return sum(data) & 0xFF


class DIPConfigApp:
    def __init__(self, root):
        self.root = root
        self.root.title("蓝牙翻谱器配置工具 V2.2")
        self.root.geometry("650x620")
        self.root.resizable(False, False)
        
        # 设备对象
        self.dev = None
        self.is_connected = False
        
        # 创建主框架
        self.main_frame = ttk.Frame(root, padding="15")
        self.main_frame.pack(fill=tk.BOTH, expand=True)
        
        # ===== 标题 =====
        title_label = ttk.Label(
            self.main_frame, 
            text="蓝牙翻谱器配置工具 V2.2", 
            font=("Arial", 16, "bold")
        )
        title_label.pack(pady=(0, 2))
        
        # 子标题
        sub_title_label = ttk.Label(
            self.main_frame,
            text="by motozilog 260728",
            font=("Arial", 9),
            foreground="gray"
        )
        sub_title_label.pack(pady=(0, 15))
        
        # ===== 设备状态 =====
        self.status_frame = ttk.LabelFrame(self.main_frame, text="设备状态", padding="8")
        self.status_frame.pack(fill=tk.X, pady=(0, 15))
        
        # 状态显示 (使用 Frame 来实现类似 C# 的布局)
        status_inner_frame = ttk.Frame(self.status_frame)
        status_inner_frame.pack()
        
        self.status_label = ttk.Label(status_inner_frame, text="未连接", foreground="red")
        self.status_label.pack(side=tk.LEFT, padx=(0, 10))
        
        # "连接" 按钮 (未连接时显示)
        self.connect_btn = ttk.Button(
            status_inner_frame, 
            text="连接", 
            command=self.manual_connect,
            width=8
        )
        self.connect_btn.pack(side=tk.LEFT)
        
        # ===== 脚踏映射配置区域 =====
        self.config_frame = ttk.LabelFrame(self.main_frame, text="脚踏映射配置", padding="10")
        self.config_frame.pack(fill=tk.BOTH, expand=True, pady=(0, 15))
        
        # 创建表格标题行
        header_frame = ttk.Frame(self.config_frame)
        header_frame.pack(fill=tk.X, pady=(0, 8))
        
        ttk.Label(header_frame, text="", width=10).grid(row=0, column=0, padx=2)
        ttk.Label(header_frame, text="左脚", font=("Arial", 9, "bold"), width=20).grid(row=0, column=1, padx=5)
        ttk.Label(header_frame, text="右脚", font=("Arial", 9, "bold"), width=20).grid(row=0, column=2, padx=5)
        
        # 存储所有 ComboBox 和 DIP 行框架的引用
        self.comboboxes = {}  # 格式: {(dip_index, 'pa5'): combobox, (dip_index, 'pa15'): combobox}
        self.row_frames = []  # 存储所有行框架，用于启用/禁用
        
        # 创建滚动容器
        canvas = tk.Canvas(self.config_frame, height=280)
        scrollbar = ttk.Scrollbar(self.config_frame, orient="vertical", command=canvas.yview)
        scrollable_frame = ttk.Frame(canvas)
        
        scrollable_frame.bind(
            "<Configure>",
            lambda e: canvas.configure(scrollregion=canvas.bbox("all"))
        )
        
        canvas.create_window((0, 0), window=scrollable_frame, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)
        
        # 创建 8 行，每行一个脚踏编号
        for i in range(8):
            row_frame = ttk.Frame(scrollable_frame)
            row_frame.pack(fill=tk.X, pady=2)
            self.row_frames.append(row_frame)
            
            # 脚踏编号标签
            ttk.Label(row_frame, text=f"{i}号", width=10).grid(row=0, column=0, padx=2, sticky="w")
            
            # 左脚 ComboBox
            left_cb = ttk.Combobox(row_frame, values=KEY_NAMES, width=22, state="disabled")
            left_cb.grid(row=0, column=1, padx=5)
            left_cb.set("Space (0x2C)")
            self.comboboxes[(i, 'left')] = left_cb
            
            # 右脚 ComboBox
            right_cb = ttk.Combobox(row_frame, values=KEY_NAMES, width=22, state="disabled")
            right_cb.grid(row=0, column=2, padx=5)
            right_cb.set("Enter (0x28)")
            self.comboboxes[(i, 'right')] = right_cb
        
        canvas.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")
        
        # ===== 底部按钮区域 =====
        self.btn_frame = ttk.Frame(self.main_frame)
        self.btn_frame.pack(fill=tk.X, pady=(5, 0))
        
        btn_container = ttk.Frame(self.btn_frame)
        btn_container.pack()
        
        self.read_btn = ttk.Button(btn_container, text="读取配置", command=self.read_config, width=12, state=tk.DISABLED)
        self.read_btn.pack(side=tk.LEFT, padx=5)
        
        self.write_btn = ttk.Button(btn_container, text="下发配置", command=self.write_config, width=12, state=tk.DISABLED)
        self.write_btn.pack(side=tk.LEFT, padx=5)
        
        self.default_btn = ttk.Button(btn_container, text="恢复出厂", command=self.restore_default, width=12, state=tk.DISABLED)
        self.default_btn.pack(side=tk.LEFT, padx=5)
        
        self.exit_btn = ttk.Button(btn_container, text="退出", command=self.root.quit, width=12)
        self.exit_btn.pack(side=tk.LEFT, padx=5)
        
        # ===== 底部状态栏 =====
        self.footer_label = ttk.Label(
            self.main_frame,
            text="就绪",
            font=("Arial", 9),
            foreground="gray"
        )
        self.footer_label.pack(side=tk.BOTTOM, pady=(10, 0))
        
        # 启动时自动连接
        self.auto_connect()
    
    def set_controls_enabled(self, enabled):
        """启用/禁用所有 ComboBox 和按钮"""
        state = "readonly" if enabled else "disabled"
        
        # 启用/禁用 ComboBox
        for key, cb in self.comboboxes.items():
            cb.config(state=state)
        
        # 启用/禁用按钮
        self.read_btn.config(state=tk.NORMAL if enabled else tk.DISABLED)
        self.write_btn.config(state=tk.NORMAL if enabled else tk.DISABLED)
        self.default_btn.config(state=tk.NORMAL if enabled else tk.DISABLED)
    
    def set_connected_state(self, connected, device_name=""):
        """更新连接状态 UI"""
        self.is_connected = connected
        
        if connected:
            self.status_label.config(text=f"已连接: {device_name}", foreground="green")
            self.connect_btn.pack_forget()  # 隐藏"连接"按钮
            self.set_controls_enabled(True)
            self.footer_label.config(text="设备已就绪")
        else:
            self.status_label.config(text="未连接", foreground="red")
            self.connect_btn.pack(side=tk.LEFT)  # 显示"连接"按钮
            self.set_controls_enabled(False)
            self.footer_label.config(text="请连接设备")
    
    def auto_connect(self):
        """自动查找并连接设备"""
        try:
            for info in hid.enumerate():
                if info['vendor_id'] == 0x1209 and info['product_id'] == 0x0001:
                    if info['usage_page'] == 0xFF00:
                        self.dev = hid.device()
                        self.dev.open_path(info['path'])
                        
                        device_name = info.get('product_string', 'USB HID Device')
                        self.set_connected_state(True, device_name)
                        
                        # 自动读取配置（静默模式，不弹窗）
                        self.auto_read_config()
                        return
            
            # 未找到设备
            self.set_connected_state(False)
            
        except Exception as e:
            self.set_connected_state(False)
            self.footer_label.config(text=f"错误: {str(e)}")
    
    def manual_connect(self):
        """手动连接 (点击"连接"按钮)"""
        self.footer_label.config(text="正在连接...")
        self.root.update()
        
        try:
            for info in hid.enumerate():
                if info['vendor_id'] == 0x1209 and info['product_id'] == 0x0001:
                    if info['usage_page'] == 0xFF00:
                        self.dev = hid.device()
                        self.dev.open_path(info['path'])
                        
                        device_name = info.get('product_string', 'USB HID Device')
                        self.set_connected_state(True, device_name)
                        
                        # 自动读取配置（静默模式，不弹窗）
                        self.auto_read_config()
                        
                        messagebox.showinfo("连接成功", f"已连接到: {device_name}")
                        return
            
            # 未找到设备
            messagebox.showwarning("连接失败", "未找到 USB HID 设备，请确保设备已插入。")
            self.set_connected_state(False)
            
        except Exception as e:
            messagebox.showerror("连接错误", f"连接设备时出错:\n{str(e)}")
            self.set_connected_state(False)
    
    def get_key_name(self, code):
        """根据键码获取显示名称"""
        if code in KEY_MAP:
            return f"{KEY_MAP[code]} (0x{code:02X})"
        return f"未知 (0x{code:02X})"
    
    def auto_read_config(self):
        """自动读取配置（静默模式，不弹窗）"""
        if self.dev is None:
            return
        
        try:
            data = self.dev.get_feature_report(0x01, 18)
            
            if data and len(data) == 18:
                dip_data = list(data[1:17])
                checksum = data[17]
                calc_sum = calc_checksum(dip_data)
                
                if checksum != calc_sum:
                    self.footer_label.config(text="自动读取失败 (校验和错误)")
                    return
                
                # 更新 ComboBox
                for i in range(8):
                    left_code = dip_data[i * 2]
                    right_code = dip_data[i * 2 + 1]
                    
                    left_name = self.get_key_name(left_code)
                    right_name = self.get_key_name(right_code)
                    
                    # 如果名称不在列表中，动态添加
                    if left_name not in self.comboboxes[(i, 'left')]['values']:
                        current_values = list(self.comboboxes[(i, 'left')]['values'])
                        current_values.append(left_name)
                        self.comboboxes[(i, 'left')]['values'] = current_values
                    
                    if right_name not in self.comboboxes[(i, 'right')]['values']:
                        current_values = list(self.comboboxes[(i, 'right')]['values'])
                        current_values.append(right_name)
                        self.comboboxes[(i, 'right')]['values'] = current_values
                    
                    self.comboboxes[(i, 'left')].set(left_name)
                    self.comboboxes[(i, 'right')].set(right_name)
                
                self.footer_label.config(text="配置自动加载成功")
            else:
                self.footer_label.config(text="自动读取失败 (数据异常)")
                
        except Exception as e:
            self.footer_label.config(text=f"自动读取错误: {str(e)}")
    
    def read_config(self):
        """从 USB 读取配置并更新 ComboBox (手动模式，有弹窗)"""
        if self.dev is None:
            messagebox.showerror("错误", "设备未连接，请先连接设备")
            return
        
        try:
            self.footer_label.config(text="正在读取配置...")
            self.root.update()
            
            data = self.dev.get_feature_report(0x01, 18)
            
            if data and len(data) == 18:
                dip_data = list(data[1:17])
                checksum = data[17]
                calc_sum = calc_checksum(dip_data)
                
                if checksum != calc_sum:
                    messagebox.showwarning("校验和错误", 
                        f"读取的数据校验和不匹配！\n"
                        f"存储校验和: 0x{checksum:02X}\n"
                        f"计算校验和: 0x{calc_sum:02X}\n\n"
                        "数据可能已损坏，建议恢复出厂设置。")
                    self.footer_label.config(text="读取完成 (校验和错误)")
                    return
                
                # 更新 ComboBox
                for i in range(8):
                    left_code = dip_data[i * 2]
                    right_code = dip_data[i * 2 + 1]
                    
                    left_name = self.get_key_name(left_code)
                    right_name = self.get_key_name(right_code)
                    
                    if left_name not in self.comboboxes[(i, 'left')]['values']:
                        current_values = list(self.comboboxes[(i, 'left')]['values'])
                        current_values.append(left_name)
                        self.comboboxes[(i, 'left')]['values'] = current_values
                    
                    if right_name not in self.comboboxes[(i, 'right')]['values']:
                        current_values = list(self.comboboxes[(i, 'right')]['values'])
                        current_values.append(right_name)
                        self.comboboxes[(i, 'right')]['values'] = current_values
                    
                    self.comboboxes[(i, 'left')].set(left_name)
                    self.comboboxes[(i, 'right')].set(right_name)
                
                # 显示读取成功的详细信息
                msg = "读取成功！\n\n脚踏映射配置:\n"
                for i in range(8):
                    left_name = self.get_key_name(dip_data[i * 2])
                    right_name = self.get_key_name(dip_data[i * 2 + 1])
                    msg += f"{i}号: 左脚={left_name}, 右脚={right_name}\n"
                
                messagebox.showinfo("读取成功", msg)
                self.footer_label.config(text="配置读取成功")
                
            else:
                messagebox.showerror("读取失败", f"读取到异常数据: {len(data) if data else 0} 字节")
                self.footer_label.config(text="读取失败")
                
        except Exception as e:
            messagebox.showerror("读取失败", f"读取配置时出错:\n{str(e)}")
            self.footer_label.config(text=f"错误: {str(e)}")
    
    def write_config(self):
        """下发配置"""
        # ===== 检查连接状态 =====
        if self.dev is None or not self.is_connected:
            # 未连接时，提示用户连接
            result = messagebox.askyesno(
                "设备未连接", 
                "设备未连接，是否现在尝试连接？"
            )
            if result:
                self.manual_connect()
                # 连接成功后继续执行下发
                if not self.is_connected:
                    return
            else:
                return
        
        try:
            # 收集当前 ComboBox 选中的值
            dip_data = []
            for i in range(8):
                left_name = self.comboboxes[(i, 'left')].get()
                right_name = self.comboboxes[(i, 'right')].get()
                
                left_code = NAME_TO_CODE.get(left_name, 0)
                right_code = NAME_TO_CODE.get(right_name, 0)
                
                dip_data.append(left_code)
                dip_data.append(right_code)
            
            # 计算校验和
            checksum = calc_checksum(dip_data)
            
            # 显示将要下发的配置
            msg = "即将下发以下配置:\n\n"
            for i in range(8):
                left_name = self.get_key_name(dip_data[i * 2])
                right_name = self.get_key_name(dip_data[i * 2 + 1])
                msg += f"{i}号: 左脚={left_name}, 右脚={right_name}\n"
            msg += f"\n校验和: 0x{checksum:02X}"
            
            if not messagebox.askyesno("确认下发", msg):
                return
            
            # ===== 执行下发 =====
            self.footer_label.config(text="正在下发配置...")
            self.root.update()
            
            # 组装 18 字节报告: [ReportID=1][16字节数据][校验和]
            report = [0x01] + dip_data + [checksum]
            
            # 发送 Feature Report
            result = self.dev.send_feature_report(report)
            
            if result < 0:
                messagebox.showerror("下发失败", f"发送失败，返回码: {result}")
                self.footer_label.config(text="下发失败")
                return
            
            # ===== 校验 =====
            self.footer_label.config(text="正在校验...")
            self.root.update()
            
            # 等待设备处理完成
            time.sleep(0.1)
            
            # 读取刚写入的数据进行校验
            verify_data = self.dev.get_feature_report(0x01, 18)
            
            if not verify_data or len(verify_data) != 18:
                messagebox.showerror("校验失败", "读取校验数据失败")
                self.footer_label.config(text="校验失败")
                return
            
            # 解析校验数据
            verify_dip = list(verify_data[1:17])
            verify_checksum = verify_data[17]
            verify_calc = calc_checksum(verify_dip)
            
            # 对比数据
            data_match = (verify_dip == dip_data)
            checksum_match = (verify_checksum == verify_calc)
            
            if data_match and checksum_match:
                messagebox.showinfo("成功", "配置下发成功！\n\n数据校验通过。")
                self.footer_label.config(text="配置下发成功")
                
                # 更新界面显示
                for i in range(8):
                    left_name = self.get_key_name(verify_dip[i * 2])
                    right_name = self.get_key_name(verify_dip[i * 2 + 1])
                    
                    if left_name not in self.comboboxes[(i, 'left')]['values']:
                        current_values = list(self.comboboxes[(i, 'left')]['values'])
                        current_values.append(left_name)
                        self.comboboxes[(i, 'left')]['values'] = current_values
                    
                    if right_name not in self.comboboxes[(i, 'right')]['values']:
                        current_values = list(self.comboboxes[(i, 'right')]['values'])
                        current_values.append(right_name)
                        self.comboboxes[(i, 'right')]['values'] = current_values
                    
                    self.comboboxes[(i, 'left')].set(left_name)
                    self.comboboxes[(i, 'right')].set(right_name)
            else:
                # 校验失败 - 显示详细对比信息
                error_msg = "校验失败！\n\n"
                error_msg += f"写入数据: {dip_data}\n"
                error_msg += f"读取数据: {verify_dip}\n\n"
                error_msg += f"写入校验和: 0x{checksum:02X}\n"
                error_msg += f"读取校验和: 0x{verify_checksum:02X}\n"
                error_msg += f"计算校验和: 0x{verify_calc:02X}\n"
                
                if not data_match:
                    # 找出不匹配的位置
                    mismatch_pos = []
                    for i in range(16):
                        if dip_data[i] != verify_dip[i]:
                            mismatch_pos.append(f"[{i}] 0x{dip_data[i]:02X} != 0x{verify_dip[i]:02X}")
                    error_msg += f"\n不匹配位置: {', '.join(mismatch_pos)}"
                
                messagebox.showerror("校验失败", error_msg)
                self.footer_label.config(text="校验失败")
                
        except Exception as e:
            messagebox.showerror("下发错误", f"下发配置时出错:\n{str(e)}")
            self.footer_label.config(text=f"错误: {str(e)}")
    
    def restore_default(self):
        """恢复出厂"""
        if self.dev is None:
            messagebox.showerror("错误", "设备未连接，请先连接设备")
            return
        
        if messagebox.askyesno("确认恢复", "确定要恢复出厂设置吗？\n这将重置所有脚踏映射为默认值。"):
            # 加载默认值
            default_data = [
                0x50, 0x4F,  # 0号: 左箭头, 右箭头
                0x52, 0x51,  # 1号: 上箭头, 下箭头
                0x4B, 0x4E,  # 2号: PageUp, PageDown
                0x2C, 0x28,  # 3号: 空格, 回车
                0x4F, 0x50,  # 4号: 右箭头, 左箭头
                0x51, 0x52,  # 5号: 下箭头, 上箭头
                0x4E, 0x4B,  # 6号: PageDown, PageUp
                0x28, 0x2C,  # 7号: 回车, 空格
            ]
            
            # 更新界面
            for i in range(8):
                left_name = self.get_key_name(default_data[i * 2])
                right_name = self.get_key_name(default_data[i * 2 + 1])
                self.comboboxes[(i, 'left')].set(left_name)
                self.comboboxes[(i, 'right')].set(right_name)
            
            self.footer_label.config(text="已恢复出厂默认值 (待下发)")
            messagebox.showinfo("恢复出厂", "已恢复出厂默认值，请点击'下发配置'写入设备。")


def main():
    root = tk.Tk()
    app = DIPConfigApp(root)
    root.mainloop()


if __name__ == "__main__":
    main()