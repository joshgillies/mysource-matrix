
package net.squiz.matrix.debug;

import java.util.*;
import java.io.*;
import net.squiz.matrix.core.*;
import javax.swing.*;
import javax.swing.table.*;

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
		
		public Message(String message, Class cls) {
			this.message = message;
			this.cls = cls;
		}
		
		public Message(String message, Class cls, Throwable t) {
			this.message = message;
			this.cls = cls;
			this.t = t;
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
	}
	
	private static String getThrowableStackTrace(Throwable t) {
		ByteArrayOutputStream bos = new ByteArrayOutputStream();
		PrintWriter pw = new PrintWriter(bos, true);
		t.printStackTrace(pw);
		
		return bos.toString();
	}
	
	public static void openLogs() {
		JFrame frame = new JFrame();
		frame.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
		
		final Message[] messagesArr = (Message[]) messages.toArray(new Message[messages.size()]);
		
		TableModel dataModel = new AbstractTableModel() {
			public int getColumnCount() {
				return 3;
			}
			public int getRowCount() {
				return messagesArr.length;
			}
			public Object getValueAt(int row, int col) {
				switch (col) {
					case 0:
						return messagesArr[row].getCls();
					case 1:
						return messagesArr[row].getMessage();
					case 2:
						if (messagesArr[row].getThrowable() != null)
							return getThrowableStackTrace(messagesArr[row].getThrowable());
				}
				return null;
			}
		};
		JTable table = new JTable(dataModel);
		frame.getContentPane().add(new JScrollPane(table));
		frame.setSize(400, 300);
		GUIUtilities.showInScreenCenter(frame);
	}
}
