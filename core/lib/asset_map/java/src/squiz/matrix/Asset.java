package squiz.matrix;

import java.util.Iterator;
import java.util.Enumeration;
import java.util.Map;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.Set;
import java.util.HashSet;
import java.util.Vector;
import java.util.Collections;

import java.awt.Color;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
 * Class for objects representing a MySource Matrix Asset. 
 * 
 * <p>
 * Responsible for keeping track of asset-related information, like the 
 * asset type, URL, web paths, etc.
 * </p>
 *
 * <p>
 * It also keeps references to <code>AssetLink</code>s this asset belongs
 * to, both parent and child. This is why it parses the link 
 * <code>Element</code> node list.
 * </p>
 * 
 * <p>
 * <code>AssetFactory</code> is responsible for instantiating 
 * <code>Asset</code>s. You shouldn't have to call the constructor for 
 * <code>Asset</code> anywhere else.
 * </p>
 *
 * <p><code>$Id: Asset.java,v 1.1 2003/11/14 05:21:36 dwong Exp $</code></p>
 * 
 * @author		Dominic Wong <dwong@squiz.net>
 * @version		$Version$
 *
 * @see			AssetFactory
 */

public class Asset {

	// member variables

	/** Asset ID */
	private int				id;

	/** Asset name */
	private String			name;

	/** 
	 * Asset type 
	 * @see AssetType
	 */
	private AssetType		type;

	/**
	 * Asset status 
	 * @see					#ARCHIVED
	 * @see					#UNDER_CONSTRUCTION
	 * @see					#PENDING_APPROVAL
	 * @see					#APPROVED
	 * @see					#LIVE
	 * @see					#LIVE_APPROVAL
	 * @see					#EDITING
	 * @see					#EDITING_APPROVAL
	 * @see					#EDITING_APPROVED
	 */
	private int				status;

	/** Whether this asset is accessible to the current user.*/
	private boolean			accessible;

	/** The URL for this asset */
	private String			url;
	/** The web paths that belong to this asset */
	private String[]		webPaths;

	/** The set of parent links, in no particular order */
	private Set				parentLinks;

	/** 
	 * The map of orders (as <code>Integer</code>s to child 
	 * <code>AssetLink</code>s.
	 */
	private SortedMap		childLinks;	


	/**
	 * Whether this asset has been loaded. Unloaded assets only possess an
	 * asset ID.
	 */
	private boolean			loaded;
	
	// some constants

	// statuses (statii is NOT a word)
	/** Asset Archival status. */
	public static final int		ARCHIVED			= 1;
	/** Asset Under Construction status. */
	public static final int		UNDER_CONSTRUCTION	= 2;
	/** Asset Workflow Pending Approval status. */
	public static final int		PENDING_APPROVAL	= 4;
	/** Asset Workflow Approved status. */
	public static final int		APPROVED			= 8;
	/** Asset Live status. */
	public static final int		LIVE				= 16;
	/** Asset Live Under Review status. */
	public static final int		LIVE_APPROVAL		= 32;
	/** Asset Safe Editing status. */
	public static final int		EDITING				= 64;
	/** Asset Workflow Approval Safe Editing status. */
	public static final int		EDITING_APPROVAL	= 128;
	/** Asset Workflow Approved Safe Editing status. */
	public static final int		EDITING_APPROVED	= 256;

	/** 
	 * Constructor. Unloaded Asset construction. 
	 * 
	 * @param id	The Asset ID
	 *
	 */
	public Asset(int id) {
		this.id = id;
		
		this.name = null;
		this.type = null;
		this.status = 0;
		this.accessible = false;
		this.url = null;
		this.webPaths = null;

		this.parentLinks = new HashSet();
		this.childLinks = new TreeMap();

		this.loaded = false;
	}//end constructor


	/**
	 * Constructor. Loaded Asset construction.
	 * 
	 * @param id			The Asset ID
	 * @param name			The name of the asset
	 * @param type			The type of the asset
	 * @param status		The status of the asset
	 * @param accessible	Whether this asset is accessible to the current 
	 *						user
	 * @param url			The URL for this asset
	 * @param webPaths		The web paths for this asset
	 * 
	 * @see					AssetType
	 * @see					#ARCHIVED				Asset.ARCHIVED
	 * @see					#UNDER_CONSTRUCTION		Asset.UNDER_CONSTRUCTION
	 * @see					#PENDING_APPROVAL		Asset.PENDING_APPROVAL
	 * @see					#APPROVED				Asset.APPROVED
	 * @see					#LIVE					Asset.LIVE
	 * @see					#LIVE_APPROVAL			Asset.LIVE_APPROVAL
	 * @see					#EDITING				Asset.EDITING
	 * @see					#EDITING_APPROVAL		Asset.EDITING_APPROVAL
	 * @see					#EDITING_APPROVED		Asset_EDITING_APPROVED
	 */
	public Asset(
			int id, 
			String name, 
			AssetType type, 
			int status, 
			boolean accessible,
			String url,
			String[] webPaths
		) {
		this.id = id;
		setInfo(name, type, status, accessible, url, webPaths);

		this.parentLinks = new HashSet();
		this.childLinks = new TreeMap();
	}//end constructor
	

	/**
	 * Compares this <code>Asset</code> against some other <code>Object</code> 
	 * for equality.
	 * 
	 * @param other			The object to compare against
	 * @return				Whether this object is equal to <code>other</code>
	 */
	public boolean equals(Object other) {
		if (!(other instanceof Asset))
			return false;
		Asset otherAsset = (Asset)other;
		return this.id == otherAsset.id();
	}//end equals()


	/** Returns the asset's ID. */
	public int id() {
		return id;
	}//end id()


	/** 
	 * Sets the asset information. Changes the <code>loaded</code> status of 
	 * this asset to <code>true</code>.
	 * 
	 * @param name			The name of the asset
	 * @param type			The type of the asset
	 * @param status		The status of the asset
	 * @param accessible	Whether this asset is accessible to the current 
	 *						user
	 * @param url			The URL for this asset
	 * @param webPaths		The web paths for this asset
	 * 
	 * @see					AssetType
	 * @see					#ARCHIVED
	 * @see					#UNDER_CONSTRUCTION
	 * @see					#PENDING_APPROVAL
	 * @see					#APPROVED
	 * @see					#LIVE
	 * @see					#LIVE_APPROVAL
	 * @see					#EDITING
	 * @see					#EDITING_APPROVAL
	 * @see					#EDITING_APPROVED
	 */
	public void setInfo(
			String name,
			AssetType type, 
			int status, 
			boolean accessible,
			String url,
			String[] webPaths
			) {
		this.name = name;
		this.type = type;
		this.status = status;
		this.accessible = accessible;
		this.url = url;
		this.webPaths = webPaths;

		this.loaded = true;

	}//end setInfo()
	

	/** 
	 * Returns whether this asset has been loaded.
	 * 
	 * @return	whether this asset has been loaded
	 */
	public boolean loaded() {
		return loaded;
	}//end loaded()


	/** 
	 * Returns the name of this asset.
	 *
	 * @return the name of this asset, or <code>null</code> if this asset is 
	 * not loaded.
	 */
	public String getName() {
		return name;
	}//end getName()


	/** 
	 * Returns the type of this asset.
	 *
	 * @return the type of this asset, or <code>null</code> if this asset is 
	 * not loaded.
	 */
	public AssetType getType() {
		return type;
	}//end getType()


	/** 
	 * Returns the status of this asset.
	 *
	 * @return the status of this asset, or <code>0</code> if this asset is 
	 * not loaded.
	 */
	public int getStatus() {
		return status;
	}//end getStatus()


	/** 
	 * Tests for accessibility of the current user for this asset. Returns 
	 * <code>false</code> if asset has not been loaded.
	 *
	 * @return whether the user has access to this asset. 
	 */
	public boolean isAccessible() {
		return accessible;
	}//end isAccessible()


	/** 
	 * Returns the URL (as a string) of this asset.
	 *
	 * @return the URL of this asset, or <code>null</code> if this asset is 
	 * not loaded.
	 */
	public String getURL() {
		return url;
	}//end getURL()


	/** 
	 * Returns the web paths for this asset.
	 *
	 * @return the web paths for this asset, or <code>0</code> if this asset is 
	 * not loaded.
	 */
	public String[] getWebPaths() {
		return webPaths;
	}//end getWebPaths()


	/** 
	 * Returns the colour of this asset's status.
	 *
	 * @param selected		Whether the asset is selected or not
	 * @return				the colour of this asset's status , or 
	 *						<code>null</code> if this asset is not loaded.
	 */
	public Color getStatusColour(boolean selected) {
		if (!loaded)
			return null;

		int rgb = 0;
		switch(status) {
			case Asset.ARCHIVED:
				if (!selected) 
					rgb = 0xA59687;
				else 
					rgb = 0x655240;
			break;

			case Asset.UNDER_CONSTRUCTION:
				if (!selected) 
					rgb = 0x78C7EB;
				else 
					rgb = 0x00A0E2;
			break;

			case Asset.PENDING_APPROVAL:
				if (!selected) 
					rgb = 0xAF9CC5;
				else 
					rgb = 0x432C5F;
			break;
			case Asset.APPROVED:
				if (!selected) 
					rgb = 0xF4D425;
				else 
					rgb = 0xEBB600;
			break;
			case Asset.LIVE:
				if (!selected) 
					rgb = 0xB1DC1B;
				else 
					rgb = 0x92B41A;
			break;
			case Asset.LIVE_APPROVAL:
				if (!selected) 
					rgb = 0xAF9CC5;
				else 
					rgb = 0x432C5F;
			break;
			case Asset.EDITING:
				if (!selected) 
					rgb = 0xF25C86;
				else 
					rgb = 0xB73E61;
			break;
			case Asset.EDITING_APPROVAL:
				if (!selected) 
					rgb = 0xCCCCCC;
				else 
					rgb = 0x666666;
			break;
			case Asset.EDITING_APPROVED:
				if (!selected) 
					rgb = 0xFF9A00;
				else 
					rgb = 0xC96606;
			break;

			default:
				return null;
		}
		return new Color(rgb);
	}//end getStatusColour()


	/**
	 * Process the XML DOM <code>NodeList</code> for this asset's links. 
	 * 
	 * @param linkElements	the XML NodeList of links for this asset, of the 
	 *						form <code>&lt;parent linkid=... /&gt;</code> or 
	 *						<code>&lt;child linkid=... /&gt;</code>
	 * @return				an array of the links found in 
	 *						<code>linkElements</code>
	 * @see					AssetFactory#processAssetsElement AssetFactory.processAssetsElement
	 */
	public AssetLink[] processLinkElements(NodeList linkElements) {
		parentLinks.clear();
		childLinks.clear();

		AssetFactory af = AssetFactory.getInstance();
		Vector newLinks = new Vector();

		for (int i = 0; i < linkElements.getLength(); ++i) {
			if (!(linkElements.item(i) instanceof Element))
				continue;

			Element linkElement = (Element)linkElements.item(i);
			int linkid		= Integer.parseInt(linkElement.getAttribute("linkid"));
			int linkType	= Integer.parseInt(linkElement.getAttribute("link_type"));
			int majorid		= Integer.parseInt(linkElement.getAttribute("majorid"));
			int minorid		= Integer.parseInt(linkElement.getAttribute("minorid"));

			if (majorid != id && minorid != id) {
				// this link doesn't have anything to do with us - should never be here
				continue;
			}
			Asset major = af.getAsset(majorid, true);
			Asset minor = af.getAsset(minorid, true);

			AssetLink newLink = new AssetLink(linkid, linkType, major, minor);
			newLinks.add(newLink);

			if (majorid == id) {
				childLinks.put(new Integer(childLinks.size()), newLink);
			}
			if (minorid == id) {
				parentLinks.add(newLink);
			}
		}

		// convert the vector into an array of AssetLink's
		AssetLink[] out = new AssetLink[newLinks.size()];
		for (int i = 0; i < newLinks.size(); ++i) {
			out[i] = (AssetLink)newLinks.get(i);
		}
		return out;
	}//end processLinkElements()


	/** Returns the number of child links for this asset. */
	public int getChildCount() {
		return childLinks.size();
	}//end getChildCount()


	/** Returns the number of parent links for this asset. */
	public int getParentCount() {
		return parentLinks.size();
	}//end getParentCount()
	

	/** 
	 * Returns an <code>Iterator</code> for iterating through the child links 
	 * in order.
	 */
	public Iterator getChildLinks() {
		return childLinks.values().iterator();
	}//end getChildLinks()
	

	/**
	 * Returns an <code>Iterator</code> for iterating through the parent links
	 * (in no particular order).
	 */
	public Iterator getParentLinks() {
		return parentLinks.iterator();
	}//end getParentLinks();


	/**
	 * Returns the <code>AssetLink</code> for a given index. 
	 * @param index		The index of the link
	 * @return			The <code>AssetLink</code>, or <code>null</code> if 
	 *					index is out of bounds
	 * @see				AssetLink
	 */
	public AssetLink getChildLinkAt(int index) {
		return (AssetLink)childLinks.get(new Integer(index));
	}//end getChildLinkAt()


	/**
	 * Returns the index for a particular child <code>AssetLink</code> of this 
	 * asset.
	 *
	 * @param childLink	The <code>AssetLink</code>, or <code>null</code> if 
	 *					index is out of bounds
	 * @return			The index of the link, or -1 if <code>childLink</code>
	 *					is not a child link of this asset
	 * @see				AssetLink
	 */
	public int getChildLinkIndex(AssetLink childLink) {
		int index = 0;
		Iterator linkIterator = getChildLinks();
		while(linkIterator.hasNext()) {
			if (childLink.equals(linkIterator.next()))
				break;
			++index;
		}

		if (linkIterator.hasNext())
			return index;
		else
			return -1;
	}//end getChildLinkIndex()


	/**
	 * Retrieves an iterator to the <i>set</i> of assets that are child linked to 
	 * this asset.
	 * 
	 * @return		an <code>Iterator</code> that iterates through the child
	 *				assets
	 */
	public Iterator getChildAssets() {
		Set children = new HashSet();
		Iterator linkIterator = getChildLinks();
		while(linkIterator.hasNext()) {
			AssetLink link = (AssetLink)linkIterator.next();
			children.add(link.getMinor());
		}
		return children.iterator();
	}//end getChildAssets()


	/**
	 * Retrieves an iterator to the <i>set</i> of assets that are parent linked to 
	 * this asset.
	 * 
	 * @return		an <code>Iterator</code> that iterates through the child
	 *				assets
	 */
	public Iterator getParentAssets() {
		Set parents = new HashSet();
		Iterator linkIterator = getParentLinks();
		while(linkIterator.hasNext()) {
			AssetLink link = (AssetLink)linkIterator.next();
			parents.add(link.getMinor());
		}
		return parents.iterator();

	}//end getParentAssets()


	/** Returns a string representation of this asset. */
	public String toString() {
		if (!loaded) {
			return "Not yet loaded Asset #" + id + " (type: unknown, id:" + id + ")";
		} else {
			return name + " (id:" + id + ", type: " + type.getTypeCode() + ")";
		}
	}//end toString()


	/** Returns a hash code for this asset, based on its ID. */
	public int hashCode() {
		return (new Integer(id).hashCode());
	}//end hashCode()

}//end class
