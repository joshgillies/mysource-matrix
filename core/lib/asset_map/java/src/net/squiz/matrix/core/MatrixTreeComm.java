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
 * $Id: MatrixTreeComm.java,v 1.2 2005/03/06 22:57:43 mmcintyre Exp $
 * $Name: not supported by cvs2svn $
 */

 /*
  * :tabSize=4:indentSize=4:noTabs=false:
  * :folding=explicit:collapseFolds=1:
  */

package net.squiz.matrix.core;

import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.assetmap.*;
import org.w3c.dom.*;
import java.io.IOException;
import javax.swing.SwingUtilities;
import net.squiz.matrix.ui.*;
import net.squiz.matrix.debug.*;
import java.net.MalformedURLException;

/**
 * MatrixTreeComm handles the request for newlink and new asset operations.
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class MatrixTreeComm implements NewLinkListener, NewAssetListener {
	
	// {{{ Public Methods
			
	/**
	 * EventListener method that is called when a request for a new link
	 * operation should be performed in the matrix system.
	 * @param evt the event
	 */
	public void requestForNewLink(final NewLinkEvent evt) {
		
		final String[] assetids = new String[] { evt.getParentNode().getAsset().getId() };
		AssetRefreshWorker worker = new AssetRefreshWorker(assetids, true) {
			public Object construct() {
				try {
					Boolean refresh = createLink(
						evt.getType(), 
						evt.getSourceNodes(),
						evt.getParentNode(), 
						evt.getIndex()
					);
					if (refresh == Boolean.TRUE) {
						String parentid = evt.getParentNode().getAsset().getId();
						return AssetManager.makeRefreshRequest(assetids);
					}
					
				} catch (IOException ioe) {
					return ioe;
				}
				return null;
			}
			public void finished() {
				// if we need to refresh the parent, call super to do so
				if (get() != null)
					super.finished();
			}
		};
		worker.start();
	}
	
	/**
	 * Event listener method that is fired when a request for a new asset to
	 * be created in the matrix system occurs.
	 * @param evt the event
	 */
	public void requestForNewAsset(NewAssetEvent evt) {
		String typeCode       = evt.getTypeCode();
		MatrixTreeNode parent = evt.getParentNode();
		int index             = evt.getIndex();
		String parentAssetid  = MatrixToolkit.rawUrlEncode(parent.getAsset().getId(), true);
		
		
		String xml = "<command action=\"get url\" cmd=\"add\" " +
					 "parent_assetid=\"" + parentAssetid + 
					 "\" pos=\"" + index + "\" type_code=\"" + 
					 typeCode + "\" />";

		Document response = null;
		MatrixStatusBar.setStatus("Requesting...");

		try {
			response = Matrix.doRequest(xml);
		} catch (IOException ioe) {
			GUIUtilities.error("Request to Matrix failed: " + ioe.getMessage(), "Error");
			MatrixStatusBar.setStatusAndClear("Request Failed!", 1000);
			Log.log("Request for new Asset failed", MatrixTreeComm.class, ioe);
			return;
		}

		MatrixStatusBar.setStatusAndClear("Success!", 1000);
		NodeList children = response.getDocumentElement().getChildNodes();
		String url = null;
		for (int i = 0; i < children.getLength(); i++) {
			if (!(children.item(i) instanceof Element))
				continue;
			Element element = (Element) children.item(i);
			if (element.getTagName().equals("url")) {
				url = element.getFirstChild().getNodeValue();
			}
		}
		try {
			AssetMap.getURL(url);
		} catch (MalformedURLException mue) {
			Log.log("Could not load new asset interface in right pane", MatrixTreeComm.class, mue);
		}
	}
	
	// }}}
	// {{{ Private Methods
	
	/**
	 * Generates the xml required to perform a link operation in the matrix system.
	 * the resulting xml is in the format:
	 * <pre>
	 *   <command action="link_type", to_parent_assetid="" to_parent_pos=""> 
	 *      <asset assetid="" linkid="" parentid="" /> 
	 *      <asset assetid="" linkid="" parentid="" /> 
	 *      <asset... /> 
	 *   </command>
	 * </pre>
	 *
	 * @param linkType the type of link
	 * @param toParentId the parent id we are linking to
	 * @param children the children affected by this link
	 * @param index the index we are linking to
	 */
	private String generateCreateLinkXML(
		String linkType,
		String toParentId,
		MatrixTreeNode[] children,
		int index) {
			StringBuffer xml = new StringBuffer();
			xml.append("<command action=\"").append(linkType).append("\"");
			xml.append(" to_parent_assetid=\"").append(toParentId).append("\"");
			xml.append(" to_parent_pos=\"").append(index).append("\">");
			
			for (int i = 0; i < children.length; i++) {
				String assetid  = MatrixToolkit.rawUrlEncode(children[i].getAsset().getId(), true);
				String linkid   = children[i].getLinkid();
				String parentid = MatrixToolkit.rawUrlEncode(
					((MatrixTreeNode) children[i].getParent()).getAsset().getId(),
					true
				);
				xml.append("<asset assetid=\"").append(assetid).append("\" ");
				xml.append(" linkid=\"").append(linkid).append("\" ");
				xml.append(" parentid=\"").append(parentid).append("\" />");
			}
			xml.append("</command>");

		return xml.toString();
	}
	
	/**
	 * Creates a link in the mysource matrix system.
	 * @param linkType the type of link to create in the matrix system
	 * @param children the children affected by the create link
	 * @param parent the parent where the link will be created
	 * @param index the index in the parent where the link will be created
	 * @return Returns Boolean.TRUE if the parent requires refreshing, 
	 * Boolean.FALSE otherwise
	 */
	private Boolean createLink(
			String linkType,
			MatrixTreeNode[] children,
			MatrixTreeNode parent,
			int index) throws IOException {
		
		// Cue tree defined a move to an unexpaned folder as index -1
		// so convert this to 0 for matrix
		if (index == -1)
			index = 0;

		String toParentId   = MatrixToolkit.rawUrlEncode(parent.getAsset().getId(), true);
		String xml          = generateCreateLinkXML(linkType, toParentId, children, index);
		Document response   = Matrix.doRequest(xml);
		NodeList childNodes = response.getDocumentElement().getChildNodes();
		
		String url = null;
		Boolean refresh = Boolean.FALSE;
		
		for (int i = 0; i < childNodes.getLength(); i++) {
			if (!(childNodes.item(i) instanceof Element))
				continue;
			Element element = (Element) childNodes.item(i);
			if (element.getTagName().equals("url")) {
				url = element.getFirstChild().getNodeValue();
			} else if (element.getTagName().equals("success")) {
				refresh = Boolean.TRUE;
			}
		}

		// if there was a url, we need to start a hipo to move the nodes
		// there were not on the same branch
		if (url != null)
			AssetMap.openWindow(url, "Create Link Hipo");
		
		return refresh;
	}
}
