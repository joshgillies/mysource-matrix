 
package net.squiz.matrix.core;

import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.assetmap.*;
import org.w3c.dom.*;
import java.io.IOException;
import javax.swing.SwingUtilities;
import net.squiz.matrix.ui.*;

public class MatrixTreeComm implements NewLinkListener, NewAssetListener {
	
	public static String POPUP_PARAMS = "toolbar=0,menubar=0,location=0" +
			",status=0,scrollbars=1,resizable=1,width=650,height=400";
	
	public void requestForNewLink(final NewLinkEvent evt) {
			SwingWorker worker = new SwingWorker() {
				public Object construct() {
					createLink(evt.getType(), evt.getSourceNodes(), evt.getParentNode(), evt.getIndex());
					return null;
				}
			};
			
			worker.start();
	}
	
	public void requestForNewAsset(NewAssetEvent evt) {
		String typeCode = evt.getTypeCode();
		MatrixTreeNode parent = evt.getParentNode();
		int index = evt.getIndex();
		
		String parentAssetid = MatrixToolkit.rawUrlEncode(parent.getAsset().getId(), true);
		
		StringBuffer xml = new StringBuffer();
		xml.append("<command action=\"get url\" cmd=\"add\" ");
		xml.append("parent_assetid=\"").append(parentAssetid);
		xml.append("\" pos=\"").append(index).append("\" type_code=\"");
		xml.append(typeCode).append("\" />");

		Document response = null;
		MatrixStatusBar.setStatus("Requesting...");

		try {
			response = Matrix.doRequest(xml.toString());
		} catch (IOException ioe) {
			GUIUtilities.error("Request to Matrix failed: " + ioe.getMessage(), "Error");
			ioe.printStackTrace();
			MatrixStatusBar.setStatusAndClear("Request Failed!", 1000);
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
		AssetMap.getURL(url);
	}
	
	
	/**
	 * <command action="move_asset", to_parent_assetid="" to_parent_pos=""> 
	 * <asset assetid="" linkid="" parentid="" /> 
	 * <asset assetid="" linkid="" parentid="" /> 
	 * <... /> 
	 * </command>
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
	
	private void createLink(
			String linkType,
			MatrixTreeNode[] children,
			MatrixTreeNode parent,
			int index) {
		
		// Cue tree defined a move to an unexpaned folder as index -1
		// so convert this to 0 for matrix
		if (index == -1)
			index = 0;
				
		System.out.println("to parent index: " + index);

		String toParentId = MatrixToolkit.rawUrlEncode(parent.getAsset().getId(), true);
		String xml = generateCreateLinkXML(linkType, toParentId, children, index);

		Document response = null;
		MatrixStatusBar.setStatus("Requesting...");

		try {
			response = Matrix.doRequest(xml);
		} catch (IOException ioe) {
			GUIUtilities.error("Request to Matrix failed: " + ioe.getMessage(), "Error");
			ioe.printStackTrace();
			MatrixStatusBar.setStatusAndClear("Request Failed!", 1000);
			return;
		}
		
		MatrixStatusBar.setStatusAndClear("Success!", 1000);
		NodeList childNodes = response.getDocumentElement().getChildNodes();
		
		System.out.println(childNodes);
		
		String url = null;
		//MM: temp hack to we get moving with model bus working
		boolean refresh = false;
		
		for (int i = 0; i < childNodes.getLength(); i++) {
			if (!(childNodes.item(i) instanceof Element))
				continue;
			Element element = (Element) childNodes.item(i);
			if (element.getTagName().equals("url")) {
				url = element.getFirstChild().getNodeValue();
			} else if (element.getTagName().equals("success")) {
				refresh = true;
				System.out.println("refreshing parent " + parent.getAsset());
				/*
				String linkid = element.getAttribute("linkid");
				System.out.println(linkid + "<------ we should be refresing this");
				// move the nodes that were on the same branch
				for (int j = 0; j < children.length; j++) {
					if (children[j].getLinkid().equals(linkid)) {
						System.out.println(children[j] + " : " + children[j].getAsset() + " moved to " + index); 
						MatrixTreeModelBus.moveNode(children[j], parent, index++);
					}
				}*/
			}
		}
		if (refresh)
			AssetManager.refreshAssets(new String[] { parent.getAsset().getId() } );

		// if there was a url, we need to start a hipo to move the nodes
		// there were not on the same brach
		if (url != null) {
			AssetMap.openWindow(url, "Moving Asset", POPUP_PARAMS);
		}
	}
}
