import javax.swing.*;
import java.awt.*;

class MatrixSubmitProgressDialog extends JFrame
{
	JProgressBar progressBar;

	MatrixSubmitProgressDialog(int max) {
    	super();
		System.out.println("---> MSBP Created");

		//Create and set up the content pane.
        //progressBar = new JProgressBar(0, max);
		getContentPane().add(new JLabel("Sending file to server..."));
        //getContentPane().add(progressBar);

		setSize(300, 200);
		setVisible(true);
	}
	
	void setValue(int val) {
		//progressBar.setValue(val);
		System.out.println("Setting value to "+val);
		repaint();
	}

	public static void main(String[] args) 
	{
		MatrixSubmitProgressDialog d = new MatrixSubmitProgressDialog(50);
	}
}
