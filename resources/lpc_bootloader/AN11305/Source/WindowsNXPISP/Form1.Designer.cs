namespace NXPISP
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.textBoxPath = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.labelPathCheck = new System.Windows.Forms.Label();
            this.labelStatusCheck = new System.Windows.Forms.Label();
            this.labelPathX = new System.Windows.Forms.Label();
            this.buttonSelect = new System.Windows.Forms.Button();
            this.labelStatusX = new System.Windows.Forms.Label();
            this.panel1 = new System.Windows.Forms.Panel();
            this.checkBoxUpdate = new System.Windows.Forms.CheckBox();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // textBoxPath
            // 
            this.textBoxPath.AllowDrop = true;
            this.textBoxPath.Font = new System.Drawing.Font("Courier New", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.textBoxPath.Location = new System.Drawing.Point(242, 48);
            this.textBoxPath.Multiline = true;
            this.textBoxPath.Name = "textBoxPath";
            this.textBoxPath.Size = new System.Drawing.Size(358, 126);
            this.textBoxPath.TabIndex = 2;
            this.textBoxPath.TextChanged += new System.EventHandler(this.textBoxPath_TextChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(242, 23);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(110, 16);
            this.label1.TabIndex = 3;
            this.label1.Text = "Firmware Path:";
            // 
            // labelPathCheck
            // 
            this.labelPathCheck.AutoSize = true;
            this.labelPathCheck.Font = new System.Drawing.Font("Wingdings", 48F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(2)));
            this.labelPathCheck.ForeColor = System.Drawing.Color.ForestGreen;
            this.labelPathCheck.Location = new System.Drawing.Point(113, 14);
            this.labelPathCheck.Name = "labelPathCheck";
            this.labelPathCheck.Size = new System.Drawing.Size(87, 71);
            this.labelPathCheck.TabIndex = 6;
            this.labelPathCheck.Text = "þ";
            this.labelPathCheck.Visible = false;
            // 
            // labelStatusCheck
            // 
            this.labelStatusCheck.AutoSize = true;
            this.labelStatusCheck.Font = new System.Drawing.Font("Wingdings", 48F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(2)));
            this.labelStatusCheck.ForeColor = System.Drawing.Color.ForestGreen;
            this.labelStatusCheck.Location = new System.Drawing.Point(111, 78);
            this.labelStatusCheck.Name = "labelStatusCheck";
            this.labelStatusCheck.Size = new System.Drawing.Size(87, 71);
            this.labelStatusCheck.TabIndex = 8;
            this.labelStatusCheck.Text = "þ";
            this.labelStatusCheck.Visible = false;
            // 
            // labelPathX
            // 
            this.labelPathX.AutoSize = true;
            this.labelPathX.Font = new System.Drawing.Font("Wingdings", 48F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(2)));
            this.labelPathX.ForeColor = System.Drawing.Color.Red;
            this.labelPathX.Location = new System.Drawing.Point(113, 14);
            this.labelPathX.Name = "labelPathX";
            this.labelPathX.Size = new System.Drawing.Size(87, 71);
            this.labelPathX.TabIndex = 7;
            this.labelPathX.Text = "ý";
            this.labelPathX.Visible = false;
            // 
            // buttonSelect
            // 
            this.buttonSelect.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.buttonSelect.Location = new System.Drawing.Point(22, 24);
            this.buttonSelect.Name = "buttonSelect";
            this.buttonSelect.Size = new System.Drawing.Size(93, 46);
            this.buttonSelect.TabIndex = 0;
            this.buttonSelect.Text = "Select Firmware";
            this.buttonSelect.UseVisualStyleBackColor = true;
            this.buttonSelect.Click += new System.EventHandler(this.buttonSelect_Click);
            // 
            // labelStatusX
            // 
            this.labelStatusX.AutoSize = true;
            this.labelStatusX.Font = new System.Drawing.Font("Wingdings", 48F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(2)));
            this.labelStatusX.ForeColor = System.Drawing.Color.Red;
            this.labelStatusX.Location = new System.Drawing.Point(113, 78);
            this.labelStatusX.Name = "labelStatusX";
            this.labelStatusX.Size = new System.Drawing.Size(87, 71);
            this.labelStatusX.TabIndex = 9;
            this.labelStatusX.Text = "ý";
            this.labelStatusX.Visible = false;
            // 
            // panel1
            // 
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panel1.Controls.Add(this.checkBoxUpdate);
            this.panel1.Controls.Add(this.buttonSelect);
            this.panel1.Controls.Add(this.labelPathX);
            this.panel1.Controls.Add(this.labelPathCheck);
            this.panel1.Controls.Add(this.labelStatusX);
            this.panel1.Controls.Add(this.labelStatusCheck);
            this.panel1.Location = new System.Drawing.Point(15, 24);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(203, 159);
            this.panel1.TabIndex = 10;
            // 
            // checkBoxUpdate
            // 
            this.checkBoxUpdate.Appearance = System.Windows.Forms.Appearance.Button;
            this.checkBoxUpdate.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.checkBoxUpdate.Location = new System.Drawing.Point(22, 88);
            this.checkBoxUpdate.Name = "checkBoxUpdate";
            this.checkBoxUpdate.Size = new System.Drawing.Size(93, 46);
            this.checkBoxUpdate.TabIndex = 10;
            this.checkBoxUpdate.Text = "Update Firmware";
            this.checkBoxUpdate.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.checkBoxUpdate.UseVisualStyleBackColor = true;
            this.checkBoxUpdate.CheckedChanged += new System.EventHandler(this.checkBoxUpdate_CheckedChanged);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(615, 204);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.textBoxPath);
            this.Name = "Form1";
            this.Text = "NXPISP";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox textBoxPath;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label labelPathCheck;
        private System.Windows.Forms.Label labelStatusCheck;
        private System.Windows.Forms.Label labelPathX;
        private System.Windows.Forms.Button buttonSelect;
        private System.Windows.Forms.Label labelStatusX;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.CheckBox checkBoxUpdate;

    }
}

