/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: MatrixTreeComm.java,v 1.14 2006/12/05 05:26:36 bcaldwell Exp $
*
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
		createLink(
			evt.getType(),
			evt.getSourceNodes(),
			evt.getParentNode(),
			evt.getIndex(),
			evt.getPrevIndex(),
			evt.getParentIds()
		);
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

		if (index >= 0) {
			if (parent.getChildCount() > 0) {
				int modifier = 0;
				if (index >= parent.getChildCount()) {
					index = parent.getChildCount()-1;
					modifier = 1;
				}
				index = (((MatrixTreeNode)parent.getChildAt(index)).getSortOrder())+modifier;
			}
		}

		String xml = "<command action=\"get url\" cmd=\"add\" " +
					 "parent_assetid=\"" + parentAssetid +
					 "\" pos=\"" + index + "\" type_code=\"" +
					 typeCode + "\" />";

		Document response = null;
		MatrixStatusBar.setStatus(Matrix.translate("asset_map_status_bar_requesting"));

		try {
			response = Matrix.doRequest(xml);
		} catch (IOException ioe) {
			Object[] transArgs = { ioe.getMessage() };
			GUIUtilities.error(Matrix.translate("asset_map_error_request_failed", transArgs), Matrix.translate("asset_map_dialog_title_error"));
			MatrixStatusBar.setStatusAndClear(Matrix.translate("asset_map_error_request_failed"), 1000);
			Log.log("Request for new Asset failed", MatrixTreeComm.class, ioe);
			return;
		}

		MatrixStatusBar.setStatusAndClear(Matrix.translate("asset_map_status_bar_success"), 1000);
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

	/**
	 * Creates a link in the matrix system and updates the new parent where
	 * appropriate. The new parent will only updated if nodes have been moved
	 * on the same branch. The updating of the GUI is swing thread safe.
	 * @param linkType the type of link to create
	 * @param children the children to move
	 * @param parent the parent to move the children under
	 * @param index the index where the childre will be placed
	 * @see NewLinkEvent.LINK_TYPE_MOVE
	 * @see NewLinkEvent.LINK_TYPE_NEW_LINK
	 * @see NewLinkEvent.LINK_TYPE_CLONE
	 */
	public static void createLink(
			final String linkType,
			final MatrixTreeNode[] children,
			MatrixTreeNode _parent,
			int _index,
			final int prevIndex,
			final String[] parentIds) {

		// Make sure ExpandingNode is not our parent
		if (_parent instanceof ExpandingNode) {
			if (_parent instanceof ExpandingNextNode) {
				_index =  AssetManager.getLimit();
			} else {
				_index =  -1;
			}
			_parent = (MatrixTreeNode)_parent.getParent();
		}

		final int index = _index;
		final MatrixTreeNode parent = _parent;

		final String[] assetids = new String[] { parent.getAsset().getId() };
		AssetRefreshWorker worker = new AssetRefreshWorker(assetids, true) {
			public Object construct() {
				try {
					int newIndex = index;
					if (!parent.getAsset().getId().equals("1")) {
						int limit = AssetManager.getLimit();
						// we need to change the index since we are not on the first set
						if (index >= limit) {
							// move the asset to the next set
							int modifier = 0;
							// check for previous node
							if (parent.getAsset().getTotalKidsLoaded() == 0) {
								modifier = 1;
							}
							newIndex = ((MatrixTreeNode)parent.getChildAt(limit-modifier)).getSortOrder() + children.length;

						} else if ((index <= 0) && (parent.getAsset().getTotalKidsLoaded() > 0)) {
							if (parent.getChildCount() > 0) {
								// move the asset to previous set
								newIndex = ((MatrixTreeNode)parent.getChildAt(1)).getSortOrder() - children.length;
							}
						} else if (index >= 0) {
							if (parent.getChildCount() > index) {
								newIndex = ((MatrixTreeNode)parent.getChildAt(index)).getSortOrder();
							} else if (parent.getChildCount() > 0) {
								newIndex = ((MatrixTreeNode)parent.getChildAt(parent.getChildCount()-1)).getSortOrder()+1;
							}
						}
					}

					Boolean refresh = doLinkRequest(
						linkType,
						children,
						parent,
						newIndex,
						parentIds
					);
					if (refresh == Boolean.TRUE) {
						String parentid = parent.getAsset().getId();
						return AssetManager.makeRefreshRequest(assetids, "");
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
				else
					MatrixStatusBar.setStatusAndClear(Matrix.translate("asset_map_status_bar_success"), 1000);
			}
		};
		worker.start();
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
	private static String generateCreateLinkXML(
		String linkType,
		String toParentId,
		MatrixTreeNode[] children,
		int index,
		String[] parentIds) {
			StringBuffer xml = new StringBuffer();
			xml.append("<command action=\"").append(linkType).append("\"");
			xml.append(" to_parent_assetid=\"").append(toParentId).append("\"");
			xml.append(" to_parent_pos=\"").append(index).append("\">");

			for (int i = 0; i < children.length; i++) {
				String assetid  = MatrixToolkit.rawUrlEncode(children[i].getAsset().getId(), true);
				String linkid   = children[i].getLinkid();
				String parentid = null;
				try {
					parentid = MatrixToolkit.rawUrlEncode(
						((MatrixTreeNode) children[i].getParent()).getAsset().getId(),
						true
					);
				} catch (NullPointerException ex) {
					parentid = parentIds[i];
				}
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
	private static Boolean doLinkRequest(
			String linkType,
			MatrixTreeNode[] children,
			MatrixTreeNode parent,
			int index,
			String[] parentIds) throws IOException {

		// Cue tree defined a move to an unexpaned folder as index -1
		// so convert this to 0 for matrix
		if (index == -1)
			index = 0;

		String toParentId   = MatrixToolkit.rawUrlEncode(parent.getAsset().getId(), true);
		String xml          = generateCreateLinkXML(linkType, toParentId, children, index, parentIds);
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
			AssetMap.openWindow(url, "");

		return refresh;
	}
}

