/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: DG.java,v 1.5 2004/06/30 05:33:28 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


package net.squiz.matrix.assetmap;

import javax.swing.JFrame;
import javax.swing.Icon;
import java.awt.*;
import javax.swing.JLabel;


/**
* debugging class, using function names from the matrix system (eg DG.bam)
*
* @author Marc McIntyre <mmcintyre@squiz.net>
*/
public final class DG {

	/** The time taken since last call to speed_check */
	private static long time = 0;

	/**
	 * Prints an object. 
	 * Assumes that there is a toString() method in the object, otherwise the default toString method in <code>Object</code> will be used
	 *
	 * @param o the object to string
	 */
	public static final void bam(Object o) {
		System.out.print(o.toString());
	}

	/**
	 * Prints an int
	 *
	 * @param i the int to print
	 */
	public static final void bam(int i) {
		System.out.print(i);
	}
	
	/**
	 * Prints a boolean 
	 *
	 * @param b the boolean to print
	 */
	public static final void bam(boolean b) {
		System.out.print(b);
	}

	/**
	 * Prints a long
	 *
	 * @param l the long to print
	 */
	public static final void bam(long l) {
		System.out.print(l);
	}

	/**
	 * Prints a double
	 *
	 * @param d the double to print
	 */
	public static final void bam(double d) {
		System.out.print(d);
	}
	
	public static void visiBam(String text) {
		JFrame f = new JFrame();
		f.getContentPane().add(new JLabel(text));
		Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
		f.setLocation(screenSize.width / 2, screenSize.height / 2);
		f.show();
	}

	/**
	 * Displays a new component in a new frame
	 * 
	 * @param c the component to print
	 */
	public static final void bam(Icon c) {
		JFrame f = new JFrame();
		f.getContentPane().add(new JLabel(c));
		Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
		f.setLocation(screenSize.width / 2, screenSize.height / 2);
		f.show();
	}

	public static final void bam(JLabel l) {
		JFrame f = new JFrame();
		f.getContentPane().add(l);
		f.show();
	}

	/**
	 * Resets the timer for speed check
	 * 
	 */
	public static void resetSpeedCheck() {
		time = 0;
	}

	/**
	 * Calculates the time taken since the last call to <code>DG.speed_check()</code>
	 */
	public static final void speedCheck() {

		if (DG.time != 0) {
			DG.bam((System.currentTimeMillis() - DG.time) / 1000.000000 + "\n");
		}
		DG.time = System.currentTimeMillis();	
	}

	/**
	 * Calculates the time taken since the last call to <code>DG.speed_check()</code>
	 * 
 	 * @param str the string to reference the speed check with
	 */
	public static final void speedCheck(String str) {
		DG.bam(str + ": ");
		DG.speedCheck();
	}


}
