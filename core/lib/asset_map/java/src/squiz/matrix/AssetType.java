package squiz.matrix;

import java.net.URL;

import javax.swing.ImageIcon;
import javax.swing.Icon;

import java.util.Enumeration;
import java.util.Iterator;

import java.util.List;
import java.util.Vector;

import java.util.Set;
import java.util.HashSet;

import java.util.Map;
import java.util.HashMap;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
 * Class for objects representing a Matrix asset type.
 * 
 * An object of this class keeps track of useful data on a particular 
 * asset type, such as its type code, name, icon, version, etc.
 * 
 * @author		Dominic Wong <dwong.squiz.net>
 * @see			AssetTypeFactory AssetTypeFactory
 */
public class AssetType {
	/** asset type code */
	private String typeCode;
	/** asset type name */
	private String name;
	/** asset type icon */
	private Icon icon;
	/** whether the asset type is instantiable */
	private boolean instantiable;
	/** asset type version */
	private String version;
	/** asset type allowed access e.g. backend_user, system_user*/
	private String allowedAccess;



	private List screens;

	private AssetType parentType;
	private Set childTypes;


	public AssetType(
			String typeCode, 
			String name, 
			boolean instantiable, 
			String version, 
			String allowedAccess, 
			URL iconURL
		) {
		this.typeCode = typeCode;
		this.name = name;
		this.icon = icon;
		this.instantiable = instantiable;
		this.version = version;
		this.allowedAccess = allowedAccess;

		if (iconURL != null)
			this.icon = new ImageIcon(iconURL);
		else
			this.icon = null;
		
		this.screens = new Vector();

		this.parentType = null;
		this.childTypes = new HashSet();
	}

	public void setParentType(AssetType parentType) {
		this.parentType = parentType;
		parentType.childTypes.add(this);
	}

	public String toString() {
		String out = new String();
		AssetType[] ancestorTypes = getAncestorTypes();
		for (int i = ancestorTypes.length - 1; i >= 0; --i) {
			out += ancestorTypes[i].getTypeCode() + ".";
		}
		out += typeCode + " [name: " + name + "; screens: " + screens.size() + "; child types: " + childTypes.size() + "]";
		return out;
	}

	public String getTypeCode() {
		return typeCode;
	}

	public String getName() {
		return name;
	}

	public Icon getIcon() {
		return icon;
	}

	public int hashCode() {
		return typeCode.hashCode();
	}

	public AssetType[] getAncestorTypes() {
		Vector lineage = new Vector();
		AssetType nextType = this;
		while (nextType.parentType != null) {
			lineage.add(nextType.parentType);
			nextType = nextType.parentType;
		}

		AssetType[] out = new AssetType[lineage.size()];
		int index = 0;
		Enumeration types = lineage.elements();
		while (types.hasMoreElements()) {
			out[index++] = (AssetType)types.nextElement();
		}
		return out;
	}

	public AssetType getParentType() {
		return this.parentType;
	}

	public void processScreenElements(NodeList screenElements) {
		for (int i = 0; i < screenElements.getLength(); ++i) {
			if (!(screenElements.item(i) instanceof Element))
				continue;
			Element screenElement = (Element)screenElements.item(i);
			String codeName = screenElement.getAttribute("code_name");
			String screenName = screenElement.getFirstChild().getNodeValue();
			screens.add(new AssetTypeScreen(codeName, screenName));
		}
	}

	private String screensToString() {
		String out = "[";
		boolean first = true;
		Iterator i = screens.iterator();
		while (i.hasNext()) {
			if (!first)
				out += ", ";
			AssetTypeScreen screen = (AssetTypeScreen)i.next();
			out += "(" + screen.codeName + "/" + screen.screenName + ")";
			first = false;
		}
		out += "]";

		return out;
	}

	public Iterator getScreens() {
		return screens.iterator();
	}

}