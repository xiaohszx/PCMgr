﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace PCMgr.WorkWindow
{
    public partial class FormDA : Form
    {
        public FormDA()
        {
            InitializeComponent();
        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            das();
        }

#if _X64_
        [DllImport(FormMain.COREDLLNAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern bool M_SU_KDA(IntPtr callback, UInt64 startaddress, UInt64 size);
#else
        [DllImport(FormMain.COREDLLNAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern bool M_SU_KDA(IntPtr callback, uint startaddress, uint size);
#endif
        [DllImport(FormMain.COREDLLNAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern bool M_SU_KDA_Test(IntPtr callback);

        private IntPtr CallbackPtr = IntPtr.Zero;
        private DACALLBACK Callback;

#if _X64_
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void DACALLBACK(UInt64 curaddress, IntPtr address, IntPtr shell, IntPtr bariny, IntPtr asm);
        private void _DACALLBACK(UInt64 curaddress, IntPtr addressstr, IntPtr shell, IntPtr bariny, IntPtr asm)
        {
            this.curaddress = curaddress;
            showedsize = curaddress - address;
            string barinystr = Marshal.PtrToStringUni(bariny);
            textBoxBariny.Text += barinystr;
            add_Item(Marshal.PtrToStringUni(addressstr), Marshal.PtrToStringUni(shell), barinystr, Marshal.PtrToStringUni(asm));
            if (showedsize == oncemaxdsize)
                add_Item("", "", FormMain.str_DblClickToDa, "conload");
        }
#else
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void DACALLBACK(UInt32 curaddress, IntPtr address, IntPtr shell, IntPtr bariny, IntPtr asm);
        private void _DACALLBACK(UInt32 curaddress, IntPtr addressstr, IntPtr shell, IntPtr bariny, IntPtr asm)
        {
            this.curaddress = curaddress;
            showedsize = curaddress - address;
            string barinystr = Marshal.PtrToStringUni(bariny);
            textBoxBariny.Text += barinystr;
            add_Item(Marshal.PtrToStringUni(addressstr), Marshal.PtrToStringUni(shell), barinystr, Marshal.PtrToStringUni(asm));
            if (showedsize == oncemaxdsize)
                add_Item("", "", FormMain.str_DblClickToDa, "conload");
        }
#endif
        private void FormDA_Load(object sender, EventArgs e)
        {
            Callback = _DACALLBACK;
            CallbackPtr = Marshal.GetFunctionPointerForDelegate(Callback);
            textBoxDesize.Text = "000000FF";
        }



#if _X64_
        private UInt64 curaddress = 0;
        private UInt64 address = 0;
        private UInt64 size = 0;
        private UInt64 showedsize = 0;
        private const UInt64 oncemaxdsize = 0xFF;
#else
        private UInt32 curaddress = 0;
        private UInt32 address = 0;
        private UInt32 size = 0;
        private UInt32 showedsize = 0;
        private const UInt32 oncemaxdsize = 0xFF;
#endif

        private void dastest()
        {
            listViewDA.Items.Clear();
            textBoxBariny.Text = "";
            M_SU_KDA_Test(CallbackPtr);
        }
        private void das()
        {
            address = 0;
            size = 0;
            showedsize = 0;
            listViewDA.Items.Clear();
            textBoxBariny.Text = "";
            if (!FormMain.MCanUseKernel())
            {
                add_Item("", "", "", FormMain.str_DriverLoadFailed);
                return;
            }
            if (textBoxTargetAddress.Text == "")
            {
                add_Item("", "", "", FormMain.str_PleaseEnterTargetAddress);
                return;
            }
            if (textBoxDesize.Text == "")
            {
                add_Item("", "", "", FormMain.str_PleaseEnterDaSize);
                return;
            }

#if _X64_
            address = Convert.ToUInt64(textBoxTargetAddress.Text, 16);
            size = Convert.ToUInt64(textBoxDesize.Text, 16);
#else
            address = Convert.ToUInt32(textBoxTargetAddress.Text, 16);
            size = Convert.ToUInt32(textBoxDesize.Text, 16);
#endif
            bool rs = false;
            if (address <= 0)
            {
                add_Item("", "", address.ToString(), "Enter address not valid.");
                return;
            }
            if (size <= 0)
            {
                add_Item("", "", size.ToString(), "Enter size not valid.");
                return;
            }
            if (size > oncemaxdsize)
                rs = M_SU_KDA(CallbackPtr, address, oncemaxdsize);
            else rs = M_SU_KDA(CallbackPtr, address, size);
            if (!rs) FormMain.LogErr("KDA Failed!");
        }

        private void add_Item(string address, string shell, string binary, string asm, string tag = null)
        {
            ListViewItem li = new ListViewItem(address);
            li.SubItems.Add(binary);
            li.SubItems.Add(shell);
            li.SubItems.Add(asm);
            li.Tag = tag;
            listViewDA.Items.Add(li);
        }

        private void listViewDA_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            if (listViewDA.SelectedItems.Count > 0)
            {
                if (listViewDA.SelectedItems[0].Tag != null && listViewDA.SelectedItems[0].Tag.ToString() == "conload")
                {
#if _X64_
                    UInt64 thissize = size - showedsize;
#else
                    UInt32 thissize = size - showedsize;
#endif
                    if (thissize > oncemaxdsize)
                        M_SU_KDA(CallbackPtr, curaddress, oncemaxdsize);
                    else M_SU_KDA(CallbackPtr, curaddress, thissize);
                }
            }
        }

        private void linkLabel1_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            dastest();
        }

        private void 复制地址ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FormMain.MCopyToClipboard2(listViewDA.SelectedItems[0].SubItems[0].Text);
        }
        private void 复制二进制码ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FormMain.MCopyToClipboard2(listViewDA.SelectedItems[0].SubItems[1].Text);
        }
        private void 复制OpCodeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FormMain.MCopyToClipboard2(listViewDA.SelectedItems[0].SubItems[2].Text);
        }
        private void 复制汇编代码ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FormMain.MCopyToClipboard2(listViewDA.SelectedItems[0].SubItems[3].Text);
        }

        private void listViewDA_MouseClick(object sender, MouseEventArgs e)
        {
            if (listViewDA.SelectedItems.Count > 0)
            {
                if (e.Button == MouseButtons.Right)
                {
                    contextMenuStrip1.Show(MousePosition);
                }
            }
        }
    }
}
