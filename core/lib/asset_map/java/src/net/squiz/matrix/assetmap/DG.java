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
* $Id: DG.java,v 1.1 2004/01/13 00:50:24 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


package net.squiz.matrix.assetmap;

import java.util.GregorianCalendar;
import java.awt.Component;

import javax.swing.JFrame;
import javax.swing.Icon;
import javax.swing.JLabel;


/**
* debugging class, using function names from the matrix system (eg DG.bam)
*
* @author Marc McIntyre <mmcintyre@squiz.net>
*/
public final class DG {

	/**
	* The time taken since last call to speed_check
	*/
	private static long time = 0;


	/**
	* Prints an object. 
	* Assumes that there is a toString() method in the object, otherwise the default toString method in <code>Object</code> will be used
	*
	* @param o the object to string
	*/
	public static final void bam(Object o) {
		System.out.println(o.toString());
	
	}//end bam()


	/**
	* Prints an int
	*
	* @param i the int to print
	*/
	public static final void bam(int i) {
		System.out.println(i);
	
	}//end bam

	
	/**
	* Prints a boolean 
	*
	* @param b the boolean to print
	*/
	public static final void bam(boolean b) {
		System.out.println(b);
	
	}//end bam()


	/**
	* Prints a long
	*
	* @param l the long to print
	*/
	public static final void bam(long l) {
		System.out.println(l);
	
	}//end bam()


	/**
	* Prints a double
	*
	* @param d the double to print
	*/
	public static final void bam(double d) {
		System.out.println(d);
	
	}//end bam()
	

	/**
	* Displays a new component in a new frame
	* 
	* @param c the component to print
	*/
	public static final void bam(Icon c) {
		JFrame f = new JFrame();
		f.getContentPane().add(new JLabel(c));
		f.show();
	
	}//end bam()


	/**
	* Resets the timer for speed check
	* 
	*/
	public static void resetSpeedCheck() {
		time = 0;

	}//end resetSpeedCheck()


	/**
	* Calculates the time taken since the last call to <code>DG.speed_check()</code>
	*/
	public static final void speedCheck() {

		GregorianCalendar cal = new GregorianCalendar();

		if (DG.time == 0) {
			DG.time = cal.getTimeInMillis();
		} else {
			DG.time = cal.getTimeInMillis() - DG.time;
			DG.bam(DG.time / 1000.000000);
		}
		
	}//end speed_check()


	/**
	* Calculates the time taken since the last call to <code>DG.speed_check()</code>
	* 
	* @param str the string to reference the speed check with
	*/
	public static final void speedCheck(String str) {
		DG.bam(str + ": ");
		DG.speedCheck();
	
	}//end speed_check()


}//end class
