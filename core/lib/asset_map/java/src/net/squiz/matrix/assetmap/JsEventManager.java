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
* $Id: JsEventManager.java,v 1.3 2004/06/30 05:20:54 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;

import net.squiz.matrix.xml.XML;

import java.util.*;
import org.w3c.dom.*;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import org.xml.sax.SAXException;
import netscape.javascript.JSObject;

/**
 * Event Manager for js Events
 *
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class JsEventManager {

	/** A list of event listners */
	private Map listenerList = new HashMap();

	/** A singleton of the jsEventManager */
	private static JsEventManager jsEventManagerInstance = null;

	/** A list of Js method names for each event */
	private Map jsCallbackMethods = new HashMap();


	/** The class name for the js event class */
	private static final String JS_EVENT_CLASS 
		= "net.squiz.matrix.assetmap.JsEvent";


	/**
	 * Returns a singleton of the <code>JsEventManager</code> class
	 *
	 * @return An Instance of the JsEventManager class
	 */
	public static JsEventManager getInstance() {
		if (JsEventManager.jsEventManagerInstance == null) {
			JsEventManager.jsEventManagerInstance = new JsEventManager();
		}
		return JsEventManager.jsEventManagerInstance;

	}

	/**
	 * Returns a singleton of the <code>JsEventManager</code> class
	 *
	 * @return An Instance of the JsEventManager class
	 */
	public static JsEventManager sharedInstance() {
		if (JsEventManager.jsEventManagerInstance == null) {
			JsEventManager.jsEventManagerInstance = new JsEventManager();
		}
		return JsEventManager.jsEventManagerInstance;

	}

	/**
	 * Adds a js event listener to listen for events
	 * 
	 * @param type the type of event to listen for
	 * @param l the listener
	 */
	public void addJsListener(String type, JsEventListener l) {

		Vector row = null;
		if (!listenerList.containsKey(type)) {
			row = (Vector) createNewListenerRow(type);
		} else {
			row = getListenersByType(type);
		}
		row.add(l);

	}

	/**
	 * Creates a new row in the listeners for a particular type of listener
	 *
	 * @param name the name to be used as the key when retreiving
	 * the list of listener for this particular event type
	 * @return the newly created row
	 */
	private Vector createNewListenerRow(String name) {
		Vector row = new Vector();
		listenerList.put(name, row);

		return row;
	
	}

	/**
	 * Returns the listeners for a particular event type
	 *
	 * @param type the type of event the listeners are listening for
	 *
	 * @return a vector of listeners
	 */
	private Vector getListenersByType(String type) {		
		return (Vector) listenerList.get(type);
	}

	/**
	 * Returns the Javascript method name that should be called for a specific 
	 * event
	 * 
	 * @param eventName the name of the event that the method name is wanted for
	 * @return the name of the javascript method, or null if there is no method 
	 * registered for this event
	 */
	private String getJsCallbackMethod(String eventName) {
		if (jsCallbackMethods.containsKey(eventName)) {
			return (String) jsCallbackMethods.get(eventName);
		}
			return null;
	}

	/**
	 * Gets called from javascript (via the applet) to initiate an event.
	 *
	 * @param type the type of event
	 * @param command the command being executed from the javascript
	 * @param params the params being passed from the javascript 
	 * (as a matrix serialised String)
	 */
	public void jsToJavaCall(String type, String command, String paramsStr) 
		throws JsEventNotFoundException {

		if (listenerList.isEmpty())
			return;
		
		// the java ver. of the javascript params
		Map params = null;

		// convert the matrix js serialised array (string) into 
		// something useful for java
		try {
			params = unpackJsArray(paramsStr);
		} catch (IOException ioe) {
			System.err.println("Could not unpack array : " + ioe.getMessage());
			return;
		}

		// make sure we have some params
		if (params == null) {
			System.err.println("Params are corrupt");
			return;
		}
		
		// add a callback function if it exists
		if (params.containsKey("callback_fn")) {
			jsCallbackMethods.put(type, (String) params.get("callback_fn"));
		}
		
		// make sure that there are some listeners in the list for this event
		if (!(listenerList.containsKey(type.toLowerCase().trim()))) {
			throw new JsEventNotFoundException("There are no " + command 
				+ " type listeners listening");
		}

		// get a list of listeners for this type of event
		Vector listeners = getListenersByType(type);
		// create a new event object for all of the listeners
		JsEvent event = new JsEvent(this, command, params);

		//loop over all the listeners for this event
		for (Enumeration e = listeners.elements(); e.hasMoreElements();) {
			EventListener l = (EventListener) (e.nextElement());
			Method eventHandler = null;

			try {
				// get the method for this particular event, based 
				// on the command name
				eventHandler = l.getClass().getDeclaredMethod(
						command, new Class [] {Class.forName(JsEventManager.JS_EVENT_CLASS)} );
			} catch (NoSuchMethodException nsme) {
				System.err.println("There is no event " + command + ": " 
						+ nsme.getMessage());
				return;
			} catch (ClassNotFoundException cnfe) {
				System.err.println("Could not find the event class " 
						+ cnfe.getMessage());
				return;
			}//end try/catch

			try {
				// invoke the method on this listener
				eventHandler.invoke(l, new Object [] {event});
			} catch (IllegalAccessException iae) {
				System.err.println("Illegal access exception when calling method "
						+ command + ": " + iae.getMessage());
			} catch (IllegalArgumentException iare) {
				System.err.println("Illegal Argument Exception when calling method "
						+ command + ": " + iare.getMessage());
			} catch (InvocationTargetException ite) {
				System.err.println("Invocation Target Exception when calling method "
						+ command + ": " + ite.getMessage());
				System.err.println("Reflection stack trace:");
				ite.getTargetException().printStackTrace();
				System.err.println("Target Exception stack trace:");
				ite.printStackTrace();
			}//end try/catch

		}//end for

	}

	/**
	 * Gets invoked to call a javascript method that coresopnds to the event type
	 *
	 * @param	event	the type of event being fired
	 * @param	args	the args to pass to the javascript array
	 */
	public void javaToJsCall(String event, Object [] args) {
		// get the DOM window
		JSObject window = JSObject.getWindow(AssetMap.getApplet());
		String jsMethod = getJsCallbackMethod(event);
		window.call(jsMethod, args);
	}

	/**
	 * Unpacks a matrix serialized <code>javascript</code> array into a 
	 * <code>HashMap</code>
	 *
	 * @param data the serialized js array 
	 * @return The java representation of the js array as a <code>HashMap</code>
	 * @throws IOException If XML parse error or malformed JS array
	 */
	public Map unpackJsArray(String data) throws IOException {

		// create a document root
		data = "<xml>" + data + "</xml>";
		Document doc = null;

		// get a document from the string
		try {
			doc = XML.getDocumentFromString(data);
		} catch (SAXException se) {
			throw new IOException("Could not parse the javascript xml Array: " 
					+ se.getMessage());
		}

		// get the root element, and the children of that root element
		Element root = doc.getDocumentElement();
		NodeList elements = root.getChildNodes();

		if (!(elements.item(0) instanceof Element)) {
			throw new IOException("Could not process the type of data structure");
		}

		// get the js data type
		Element type = (Element) elements.item(0);

		// check to see if we have data
		if (type == null)
			return null;
		// check to make sure that the first element is the data type
		if (!(type.getTagName().equals("val_type")))
			throw new IOException("Expected val_type, got: " + type.getTagName());
		// check to make sure that this js data type is an array
		if (!(type.getFirstChild().getNodeValue().equals("array")))
			throw new IOException(
					"Expected array for val_type, got: " 
					+ type.getFirstChild().getNodeValue());
		
		// create a hashmap to store this js array in java form
		Map jsArray = new HashMap();

		String name = null;
		String val = null;
		boolean haveName = false;
		boolean haveVal = false;

		// loop over the elements an find the name/value pairs
		for (int i = 1; i < (elements.getLength()); i++) {
			// we only want elements
			if (!(elements.item(i) instanceof Element))
				continue;
			
			Element element = (Element) elements.item(i);
			// we are looking for the key and value pairs of the array
			if (element.getTagName().equals("name")) {
				haveName = true;
				name = element.getFirstChild().getNodeValue();
			} else if (element.getTagName().equals("val")) {
				haveVal = true;
				// make sure that we have something to store
				if (element.hasChildNodes()) {
					val = element.getFirstChild().getNodeValue();
				} else {
					val = "";
				}
			}
			
			if (haveName && haveVal) {
				jsArray.put(name, val);
				haveName = false;
				haveVal = false;
			}
		}//end for

		return jsArray;
		
	}
}