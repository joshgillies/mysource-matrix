package squiz.matrix;

/** 
 * An exception indicating that an asset type was not found.
 * 
 * <code>$Id: AssetTypeNotFoundException.java,v 1.1 2003/11/14 05:21:36 dwong Exp $</code>
 *
 * @author Dominic Wong <dwong@squiz.net>
 * 
 * @see AssetTypeFactory
 */
public class AssetTypeNotFoundException extends Exception { 
	/** Constructor */
	public AssetTypeNotFoundException() {
		super();
	}//end constructor

	/** Constructor with message.*/
	public AssetTypeNotFoundException(String msg) {
		super(msg);
	}//end constructor
}//end class

