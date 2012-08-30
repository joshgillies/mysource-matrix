/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: Log.java,v 1.5 2012/08/30 01:09:20 ewang Exp $
*
*/

package net.squiz.matrix.debug;

import java.util.*;
import java.io.*;
import net.squiz.matrix.core.*;
import javax.swing.*;
import javax.swing.table.*;
import java.awt.event.*;
import java.awt.*;
import java.text.DateFormat;

public class Log {

	private static Vector messages = new Vector();

	// cannot instantiate
	private Log() {}

	public static void log(String message, Class originator) {
		messages.add(new Message(message, originator));
	}

	public static void log(String message, Class originator, Throwable t) {
		messages.add(new Message(message, originator, t));
	}

	public static class Message {
		private String message;
		private Class cls;
		private Throwable t;
		private Date date;

		public Message(String message, Class cls) {
			this.message = message;
			this.cls = cls;
			date = new Date();
		}

		public Message(String message, Class cls, Throwable t) {
			this.message = message;
			this.cls = cls;
			this.t = t;
			date = new Date();
		}

		public String toString() {
			return  cls + " - " + message;
		}

		public String getCls() {
			return cls.getName();
		}

		public String getMessage() {
			return message;
		}

		public Throwable getThrowable() {
			return t;
		}

		public Date getDate() {
			return date;
		}
	}

	private static String getThrowableStackTrace(Throwable t) {
		if (t == null)
			return "";
		ByteArrayOutputStream bos = new ByteArrayOutputStream();
		PrintWriter pw = new PrintWriter(bos, true);
		t.printStackTrace(pw);

		return bos.toString();
	}

	private static String getMessagesAsText() {
		Iterator iterator = messages.iterator();

		String str = "";
		while (iterator.hasNext()) {
			Message message = (Message) iterator.next();
			str += message.getDate() + "\t";
			str += message.getCls() + "\t";
			str += message.getMessage() + "\n";
			str += getThrowableStackTrace(message.getThrowable());
			str += "-------------------------------------------\n";
		}
		return str;
	}

	public static void openLogs() {
		final JFrame frame = new JFrame();
		JPanel buttonPanel = new JPanel();
		frame.getContentPane().setLayout(new BorderLayout());
		buttonPanel.setLayout(new FlowLayout(FlowLayout.CENTER));

		ActionListener exportListener = new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				JTextPane textPane = new JTextPane();
				textPane.setEditable(false);
				textPane.setText(getMessagesAsText());

				JFrame frame = new JFrame();
				frame.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
				frame.getContentPane().add(new JScrollPane(textPane));
				frame.setSize(600, 400);

				GUIUtilities.showInScreenCenter(frame);
			}
		};

		ActionListener closeListener = new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				frame.dispose();
			}
		};

		JButton exportButton = new JButton(Matrix.translate("asset_map_button_export"));
		JButton closeButton  = new JButton(Matrix.translate("asset_map_button_close"));
		exportButton.addActionListener(exportListener);
		closeButton.addActionListener(closeListener);

		buttonPanel.add(exportButton);
		buttonPanel.add(closeButton);

		final Message[] messagesArr = (Message[]) messages.toArray(new Message[messages.size()]);

		TableModel dataModel = new AbstractTableModel() {

			private String [] columns = {
				Matrix.translate("asset_map_log_column_id"),
				Matrix.translate("asset_map_log_column_date"),
				Matrix.translate("asset_map_log_column_class"),
				Matrix.translate("asset_map_log_column_message"),
				Matrix.translate("asset_map_log_column_exception"),
			};

			public int getColumnCount() {
				return columns.length;
			}
			public int getRowCount() {
				return messagesArr.length;
			}
			public Object getValueAt(int row, int col) {
				switch (col) {
					case 0:
						return new Integer(row);
					case 1:
						return messagesArr[row].getDate();
					case 2:
						return messagesArr[row].getCls();
					case 3:
						return messagesArr[row].getMessage();
					case 4:
						if (messagesArr[row].getThrowable() != null)
							return getThrowableStackTrace(messagesArr[row].getThrowable());
				}
				return null;
			}

			public String getColumnName(int columnIndex) {
				return columns[columnIndex];
			}
		};
		JTable table = new JTable(dataModel);
		frame.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
		frame.getContentPane().add(new JScrollPane(table), BorderLayout.CENTER);
		frame.getContentPane().add(buttonPanel, BorderLayout.SOUTH);
		frame.setSize(400, 300);

		GUIUtilities.showInScreenCenter(frame);
	}
}
