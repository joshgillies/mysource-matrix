import javax.swing.*;
import java.awt.*;

class MatrixSubmitProgressDialog extends JFrame
{
	JProgressBar progressBar;

	/**
	* Constructor
	*
	* @access private
	* @return void
	*/
	MatrixSubmitProgressDialog(int max)
	{
		super();
		getContentPane().add(new JLabel("Sending file to server..."));
		setSize(300, 200);
		Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
		setLocation(screen.width - 300, (screen.height / 2) - 200);

		setVisible(true);

	}//end MatrixSubmitProgressDialog()


	/**
	* Set the value of the progress bar
	*
	* @access private
	* @return void
	*/
	void setValue(int val)
	{
		repaint();

	}//end setValue()


}//end class