package ij;
import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.net.*;
import ij.plugin.JpegWriter;
import java.io.*;
import java.util.*;
import ij.io.*;
import java.beans.*; //property change stuff

public class ServerSubmitter implements ActionListener
{
	public static final String CONFIRM_PREFIX = "OK";
	public static final String ERROR_PREFIX = "ERROR";

	/* The ImageJ instance we were fired from */
	private ImageJ ij;

	/* How many bytes we've uploaded, or -1 if we're finished and have received the server response */
	private int progress = 0;

	private String tempFileName = null;

	/**
	* Constructor
	*/
	ServerSubmitter(ImageJ ij)
	{
		this.ij = ij;

	}//end ServerSubmitter()


	/**
	* Handle the clicking of the Submit button
	*/
	public void actionPerformed(ActionEvent ev)
	{

		// get the file data to send
		byte[] fileData = null;
		String fileType = ij.getFileType();
		if (fileType == ".gif") {
			fileData = getGif(ij.getImagePlus());
		} else if (fileType == ".jpg") {
			fileData = getJpeg(ij.getImagePlus());
		} else {
			showError("Undefined file type " + fileType);
		}
		String fileName = ij.getFilename();
		if (fileName.length() < 5) {
			showError("You must give a filename before committing");
			return;
		}
		final int length = fileData.length;

		// Figure out where to send it
		URL submitURL = null;
		try {
			submitURL = new URL(ij.getParameter("SUBMIT_URL"));
		} catch (MalformedURLException e) {
			showError("Malformed URL: "+ij.getParameter("SUBMIT_URL"));
			return;
		} catch (Exception e) {
			showError("Problem getting SUBMIT_URL parameter");
			return;
		}

		// Start showing progress
		Thread progressThread = new Thread() {
			public void run() {
				ServerSubmitProgressDialog progressDialog = new ServerSubmitProgressDialog(length);
				int p = getProgress();
				while (p < length) {
					progressDialog.setValue(p);
					try { sleep(50); } catch (Exception e) {}
					p = getProgress();
				}
				progressDialog.setTitle("Waiting for server response...");
				progressDialog.goIndeterminate();
				while (progress != -1) {
					try { sleep(100); } catch (Exception e) {}
				}
				progressDialog.setVisible(false);
			}
		};
		progressThread.start();

		// Start sending the data
		ServerSubmitterThread submitterThread = new ServerSubmitterThread(this, fileData, fileName, submitURL);
		submitterThread.start();

	}//end actionPerformed()


	/**
	* Get one of the applet's parameters
	*/
	public String getParameter(String name)
	{
		return ij.getParameter(name);
	}


	/**
	* Get GIF data for the specified ImagePlus
	*/
	public byte[] getGif(ImagePlus imp)
	{
		try {
			FileInfo fi = imp.getFileInfo();
			byte[] pixels = (byte[])imp.getProcessor().getPixels();
			GifEncoder encoder = new GifEncoder(fi.width, fi.height, pixels, fi.reds, fi.greens, fi.blues);
			ByteArrayOutputStream baos = new ByteArrayOutputStream();
			OutputStream output = new BufferedOutputStream(baos);
			encoder.write(output);
			return baos.toByteArray();
		}
		catch (IOException e) {
			showError(e.getMessage());
			return null;
		}

	}//end getGif()


	/**
	* Get JPEG data for the specified ImagePlus
	*/
	public byte[] getJpeg(ImagePlus imp)
	{
		JpegWriter jpr = new JpegWriter();
		return jpr.getJpegContents(imp);

	}//end getJpeg()


	/**
	* Show an error dialog
	*/
	void showError(String msg)
	{
		JOptionPane.showMessageDialog(ij, msg, "Upload Error", JOptionPane.ERROR_MESSAGE);

	}//end showError()


	/**
	* Set the displayed progress
	*/
	synchronized void setProgress(int val)
	{
		progress = val;

	}//end setProgress()


	/**
	* Get the progress to display
	*/
	synchronized int getProgress()
	{
		return progress;

	}//end setProgress()


	/**
	* Hide the progress indicator, redirect the browser to the supplied URL
	*/
	synchronized void finish(String tempFileName)
	{
		progress = -1;
		this.tempFileName = tempFileName;

	}//end finish()

	String getTempFileName()
	{
		while (getProgress() != -1) try { Thread.sleep(333); } catch (Exception e) {}
		return this.tempFileName;
	}



}//end class



class ServerSubmitterThread extends Thread
{
	/* The ServerSubmitter that created us */
	ServerSubmitter ms;

	/* The data to upload */
	byte[] fileData;

	/* What to call the uploaded file */
	String fileName;

	/* The string to use to mark the boundaries between parts of the HTTP message */
	String boundary;

	/* The URL to submit to */
	URL submitURL;


	/**
	* Constructor
	*/
	ServerSubmitterThread(ServerSubmitter ms, byte[] fileData, String fileName, URL submitURL)
	{
		this.ms = ms;
		this.fileData = fileData;
		this.fileName = fileName;
		this.submitURL = submitURL;
		this.boundary = getBoundary();

	}//end ServerSubmitterThread


	/**
	* Get the string that represents a POST field in the HTTP message
	*/
	private String getPostComponent(String name, String val)
	{
		return "\r\n--"+boundary+"\r\nContent-Disposition: form-data; name=\""+name+"\"\r\n\r\n"+val;

	}//end getPostComponent()


	/**
	* Do the uploading
	*/
	public void run()
	{
		// get the file component head
		String fileFieldName = ms.getParameter("FILE_FIELD_NAME");
		if (fileFieldName == null) fileFieldName = "file_0";
		StringBuffer fileHeader = new StringBuffer();
		fileHeader.append("--" + boundary + "\r\n");
		fileHeader.append("Content-Disposition: form-data; name=\""+fileFieldName+"\"; fileName=\""+fileName+"\"\r\n");
		fileHeader.append("Content-Type: application/octet-stream");
		fileHeader.append("\r\n\r\n");

		// get the other POST components and the tail
		StringBuffer tail = new StringBuffer();
		int i = 0;
		String fieldName = ms.getParameter("FIELD_NAME_"+i);
		String fieldValue = ms.getParameter("FIELD_VALUE_"+i);
		while (fieldName != null) {
			//ms.showError("Field "+fieldName+"   set to  "+fieldValue);
			tail.append(getPostComponent(fieldName, fieldValue));
			i++;
			fieldName = ms.getParameter("FIELD_NAME_"+i);
			fieldValue = ms.getParameter("FIELD_VALUE_"+i);
		}
		tail.append("\r\n--");
		tail.append(boundary);
		tail.append("--\r\n");

		// get the request header
		StringBuffer header = new StringBuffer();
		header.append("POST "+submitURL.getPath()+"?"+submitURL.getQuery()+" HTTP/1.1\r\n");
		header.append("Host: "+submitURL.getHost()+"\r\n");
		header.append("Content-type: multipart/form-data; boundary="+boundary+"\r\n");
		header.append("Content-length: "+((int)(fileData.length + fileHeader.length() + tail.length()))+"\r\n");
		header.append("Connection: Keep-Alive\r\n");
		header.append("\r\n");
		//JOptionPane.showMessageDialog(ms.ij, "HEADER: "+header);

		Socket sock = null;
		DataOutputStream dataout = null;
		BufferedReader datain = null;

		try {

			sock = new Socket(submitURL.getHost(), (-1 == submitURL.getPort())?80:submitURL.getPort());
			dataout = new DataOutputStream(new BufferedOutputStream(sock.getOutputStream()));
			datain  = new BufferedReader(new InputStreamReader(sock.getInputStream()));

			// send the header
			dataout.writeBytes(header.toString());

			// send the file
			dataout.writeBytes(fileHeader.toString());
			byte[] byteBuff = new byte[1024];
			int numBytes = 0;
			int totalBytes = 0;
			ByteArrayInputStream is = new ByteArrayInputStream(fileData);
			while(-1 != (numBytes = is.read(byteBuff))) {
				//JOptionPane.showMessageDialog(ij, "Wrote "+numBytes+" bytes");
				dataout.write(byteBuff, 0, numBytes);
				totalBytes += numBytes;
				ms.setProgress(totalBytes);
				try { Thread.sleep(20); } catch (Exception e) {}
			}

			// wind it up
			dataout.writeBytes(tail.toString());
			dataout.flush();

		} catch (Exception e) {
			ms.showError("Error while writing to server: "+e.getMessage());
			e.printStackTrace();
			return;
		}

		try {
			// get the server's response
			String line = datain.readLine();
			String confirmLine = null;
			URL redirectLocation;
			String error = null;
			//StringBuffer sb = new StringBuffer();
			while ((line != null)) {
				//sb.append(line + "\n");
				if (line.indexOf("403 Forbidden") != -1) {
					error = "Server returned 403 forbidden.  You don't seem to have access to the system";
					break;
				}
				if (line.indexOf("MySource Warning") != -1) {
					error = "MySource Warning: \n" + getMysourceMessage(datain);
					break;
				}
				if (line.indexOf("MySource Error") != -1) {
					error = "MySource Error: \n" + getMysourceMessage(datain);
					break;
				}
				if (line.trim().startsWith(ServerSubmitter.CONFIRM_PREFIX)) {
					confirmLine = line.trim();
					break;
				}
				if (line.trim().startsWith(ServerSubmitter.ERROR_PREFIX)) {
					error = line;
					confirmLine = "";
					break;
				}
				line = datain.readLine();
			}
			//ms.showError(sb.toString());

			if (error != null) {
				ms.showError(error);
				ms.finish(null);
				return;
			}
			if (confirmLine == null) {
				ms.showError("No confirm line found");
				ms.finish(null);
				return;
			}
			ms.finish(confirmLine.substring(ms.CONFIRM_PREFIX.length()+1));
		} catch (Exception e) {
			ms.showError("Error while reading server response: "+e.getMessage());
			e.printStackTrace();
			ms.finish(null);
			return;
		}

	}//end run()

	private String getMysourceMessage(BufferedReader datain)
	{
		StringBuffer msg = new StringBuffer();
		try {
			String line = datain.readLine();
			while (line.indexOf("File:</td>") == -1) {
				line = datain.readLine();
			}
			line = datain.readLine(); // file line
			line = line.substring(0, line.lastIndexOf("</td>")); // strip close td
			line = line.substring(line.lastIndexOf(">")+1); // strip everything before the filename
			msg.append("File:  ");
			msg.append(line);
			msg.append("    ");

			while (line.indexOf("Line:</td>") == -1) {
				line = datain.readLine();
			}
			line = datain.readLine(); // line line
			line = line.substring(0, line.lastIndexOf("</td>")); // strip close td
			line = line.substring(line.lastIndexOf(">")+1); // strip everything before the line
			msg.append("Line:  ");
			msg.append(line);
			msg.append("\n");

			while (line.indexOf("Message:</td>") == -1) {
				line = datain.readLine();
			}
			line = datain.readLine(); // message line
			line = line.substring(0, line.lastIndexOf("</td>")); // strip close td
			line = line.substring(line.lastIndexOf(">")+1); // strip everything before the message
			msg.append("Message:  ");
			msg.append(line.replaceAll("&quot;", "\""));
		} catch (Exception e) {
			msg.append("...Exception reading error response...");
		}
		return msg.toString();
	}



	/**
	* Get the random string to use as a boundary between message parts
	*/
	private String getBoundary()
	{
		char[] allChars = new String("1234567890abcdefghijklmnopqrstuvwxyz").toCharArray();
		int len = allChars.length - 1;
		return "-----------------------------"
			+ allChars[(int)(Math.random() * len)]
			+ allChars[(int)(Math.random() * len)]
			+ allChars[(int)(Math.random() * len)]
			+ allChars[(int)(Math.random() * len)]
			+ allChars[(int)(Math.random() * len)]
			+ allChars[(int)(Math.random() * len)]
			+ allChars[(int)(Math.random() * len)]
			+ allChars[(int)(Math.random() * len)]
			+ allChars[(int)(Math.random() * len)]
			+ allChars[(int)(Math.random() * len)]
			+ allChars[(int)(Math.random() * len)];

	}//end getBoundary()


}//end class



class ServerSubmitProgressDialog extends JFrame
{
	/* Progress bar to show */
	JProgressBar progressBar;

	/**
	* Constructor - create and show the dialog
	*
	* @param	max		The maximum value for the progressbar
	*/
	ServerSubmitProgressDialog(int max)
	{
		super();
		progressBar = new JProgressBar(0, max);
		progressBar.setStringPainted(true);
		progressBar.setValue(0);
		getContentPane().add(progressBar);
		setSize(400, 80);
		validate();
		Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
		setLocation((screen.width /	2) - 200, (screen.height / 2) -	40);
		setVisible(true);
		toFront();
		repaint();

	}//end ServerSubmitProgressDialog()


	/**
	* Set the value shown by the ProgressBar
	*/
	void setValue(int val)
	{
		progressBar.setValue(val);
		repaint();

	}//end setValue()


	/**
	* Put the ProgressBar into indeterminate mode (ie something is happening but no progress shown)
	*/
	void goIndeterminate()
	{
		progressBar.setIndeterminate(true);
		progressBar.setStringPainted(false);

	}//end goIndeterminate()



}//end class
