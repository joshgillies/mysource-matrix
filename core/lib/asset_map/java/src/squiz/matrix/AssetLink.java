package squiz.matrix;

/**
 * Class for objects representing a MySource Matrix Asset Link
 *
 * @author		Dominic Wong<dwong@squiz.net>
 * @see Asset
 */

public class AssetLink {
	/**  The link ID */
	private int id;
	/**  The parent/major asset */
	private Asset majorAsset;
	/**  The child/minor asset */
	private Asset minorAsset;
	/**  The link type */
	private int linkType;

	/**  TYPE 1 link - visible in the frontend and backend */
	public static final int TYPE_1		= 1;
	/**  TYPE 2 link - visible only in the backend */
	public static final int TYPE_2		= 2;
	/**  TYPE_3 link - invisible in both frontend and backend */
	public static final int TYPE_3		= 4;
	/**  TYPE_NOTICE link - notice link */
	public static final int TYPE_NOTICE = 8;

	/**  
	 * Constructor.
	 *
	 * @param linkid		the link ID
	 * @param linkType		the link type, one of TYPE_1, TYPE_2, TYPE_3 or TYPE_NOTICE
	 * @param majorAsset	the major asset
	 * @param minorAsset	the minor asset
	 * @see					#TYPE_1
	 * @see					#TYPE_2
	 * @see					#TYPE_3
	 * @see					#TYPE_NOTICE
	 */
	public AssetLink(int linkid, int linkType, Asset majorAsset, Asset minorAsset) {
		this.id = linkid;
		this.linkType = linkType;
		this.majorAsset = majorAsset;
		this.minorAsset = minorAsset;
	}//end constructor

	/**
	 * Returns the ID of this link.
	 *
	 * @return		the link ID
	 */
	public int id() { 
		return id; 
	}//end id()

	/**
	 * Returns the major/parent asset of this link.
	 * 
	 * @return			the major asset
	 */
	public Asset getMajor() {
		return this.majorAsset;
	}//end getMajor()

	/**
	 * Returns the minor/child asset of this link.
	 * 
	 * @return			the major asset
	 */
	public Asset getMinor() {
		return this.minorAsset;
	}//end getMinor()

	/**
	 * Tests this link against an arbitrary object for equality.
	 * 
	 * @param			The object to test against
	 * @return			returns <code>true</code> if this link is the same as 
	 *					<code>o</code>, <code>false</code> otherwise
	 */
	public boolean equals(Object o) {
		if (!(o instanceof AssetLink))
			return false;
		AssetLink l = (AssetLink)o;
		return id == l.id();
	}//end equals()

	/**
	 * Returns a string representation of this link.
	 * 
	 * @return			String representation of this link
	 */
	public String toString() {
		
		return "Link #" + id + ": " + majorAsset + " -> " + minorAsset;
	}//end toString()

}//end class