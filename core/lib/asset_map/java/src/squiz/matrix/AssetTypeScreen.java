package squiz.matrix;

/**
 * An object that represents a screen for an <code>AssetType</code>.
 * 
 * <code>$Id: AssetTypeScreen.java,v 1.1 2003/11/14 05:21:36 dwong Exp $</code>
 * 
 * @author	Dominic Wong <dwong@squiz.net>
 * @see		AssetType
 */ 
public class AssetTypeScreen
{
	/** the code name for this screen */
	public String codeName;
	/** the screen name for this screen */
	public String screenName;

	/**
	 * Constructor
	 * @param codeName		the screen code name
	 * @param screenName	the screen's pretty name
	 */
	public AssetTypeScreen(String codeName, String screenName) {
		this.codeName = codeName;
		this.screenName = screenName;
	}//end constructor

}//end class
