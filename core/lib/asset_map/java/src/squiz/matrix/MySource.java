package squiz.matrix;

import java.net.URL;
import java.net.URLConnection;
import java.net.MalformedURLException;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;

import org.xml.sax.SAXException;
import java.io.ByteArrayOutputStream;
import java.io.PrintWriter;
import java.io.IOException;

/**
 * An interface to the MySource system. There is only one interface, 
 * accessible through the static method <code>getInstance()</code>.
 *
 * <p>
 * Clients of this class call <code>doRequest</code> to send messages
 * to MySource, which is controlled on that side by 
 * <code>core/lib/asset_map/asset_map.inc</code>.
 * </p>
 * 
 * <p>
 * This would be where we broadcast our pirate signal and hack into 
 * the Matrix...
 * </p>
 */
public class MySource {
	/** The unique instance of the MySource object. */
	private static MySource instance = null;

	/** base URL for the Matrix system */
	private URL baseURL;
	/** execution URL - URL for talking to the asset map */
	private URL execURL;

	/** 
	 * Gets the unique instance of the MySource object. 
	 * 
	 * @return the <code>MySource</code> singleton instance
	 */
	public static MySource getInstance() {
		if (MySource.instance == null) {
			MySource.instance = new MySource(
				"http://beta.squiz.net/dom_resolvefx", 
				"http://beta.squiz.net/dom_resolvefx/_edit/?SQ_BACKEND_PAGE=asset_map_request"
			);
		}
		return MySource.instance;
	}//end getInstance()

	/**
	 * Constructor.
	 *
	 * @param baseURLString The base URL for the Matrix system
	 * @param execURLString The URL for the PHP asset map object of the Matrix
	 */
	private MySource(String baseURLString, String execURLString) {
		try {
			baseURL = new URL(baseURLString);
			execURL = new URL(execURLString);
		} catch (MalformedURLException mue) {
			System.err.println ("Could not set url : " + mue.getMessage());
			baseURL = null;
			execURL = null;
		}
	}//end constructor

	/** 
	 * Returns the base URL for the Matrix system.
	 * 
	 * @return		the base URL for the Matrix system, or <code>null</code>
	 *				if we don't have one.
	 */
	public URL getBaseURL() {
		if (baseURL != null) {
			try {
				return new URL(baseURL.toString());
			} catch (MalformedURLException mue) {
				return null;
			}
		}
		return null;
	}//end getBaseURL()

	/**
	 * Retrieves the XML DOM parser for parsing results from 
	 * <code>asset_map.inc</code>
	 * @return		the parser, or <code>null</code> if the parser factory is 
	 *				not configured correctly
	 */
	private DocumentBuilder getParser() {
		try {
			DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
			factory.setIgnoringElementContentWhitespace(false);
			DocumentBuilder out = factory.newDocumentBuilder();
			return out;
		} catch (ParserConfigurationException pce) {
			return null;
		}
	}//end getParser()

	/**
	 * Performs an XML request to <code>asset_map.inc</code>.
	 * 
	 * @param request		The String of the XML request
	 * @return	the XML DOM <code>Document</code> response from the Matrix
	 * @throws IOException	if connection can't be opened, XML parse error, or an error 
	 *						given by the Matrix
	 */
	public Document doRequest(String request) throws IOException {
		if (execURL == null) {
			throw new IOException("ExecURL is null");
		}

		URLConnection conn = null;
		try {
			conn = execURL.openConnection();
			conn.setUseCaches(false);
			conn.setDoOutput(true);
			ByteArrayOutputStream byteStream = new ByteArrayOutputStream(512); // Grows if necessary
			PrintWriter out = new PrintWriter(byteStream, true);
			String postData = request;
			out.print(postData);
			out.flush();

			conn.setRequestProperty("Content-Length", String.valueOf(byteStream.size()));
			conn.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");

			byteStream.writeTo(conn.getOutputStream());
		} catch (IOException ioe) {
			throw new IOException ("error while getting request connection : " + ioe.getMessage());
		}

		Document document = null;

		DocumentBuilder builder = getParser();
		if (builder == null) {
			throw new IOException("error while getting document builder - parser configuration exception");
		}	
		
		try {
			document = builder.parse(conn.getInputStream()); 
		} catch (SAXException se) {
			throw new IOException("error while parsing : \n" + se.getMessage());
		} 

		if (document.getDocumentElement().getTagName().equals("error")) {
			Element errorElement = (Element)document.getDocumentElement();
			throw new IOException("error while getting response - error returned: " + errorElement.getFirstChild().getNodeValue());
		}

		return document;
	}

	private static void printDocument(Document d) {
		Element documentElement = d.getDocumentElement();
		NodeList childNodes = documentElement.getChildNodes();
		
		// see if we have an error element first
		if (documentElement.getTagName().equals("error"))
			printErrorElement(documentElement);

		for (int i = 0; i < childNodes.getLength(); ++i) {
			Node childNode = childNodes.item(i);
			if (childNode instanceof Element) {
				Element childElement = (Element)childNode;
				System.err.println (childElement.getTagName());
				printElement(childElement);
			}
		}
	}

	private static void printElement(Element e) {
		System.out.println (e.getTagName());
		if (e.getTagName().equals("error")) {
			printErrorElement(e);
		} else if (e.getTagName().equals("assets")) {
			printAssetsElement(e);
		}
	}

	private static void printAssetsElement(Element assetsElement){
		NodeList childNodes = assetsElement.getChildNodes();
		for (int i = 0; i < childNodes.getLength(); ++i) {
			Node childNode = childNodes.item(i);
			if (childNode instanceof Element) {
				Element assetElement = (Element)childNode;
				printAssetElement(assetElement);
			}
		}
	}

	private static void printAssetElement(Element assetElement) {
		System.out.println("asset " + assetElement.getAttribute("assetid") + ":" + assetElement.getAttribute("name") + " (" + assetElement.getAttribute("status") + ")");
		NodeList childNodes = assetElement.getChildNodes();
		for (int i = 0; i < childNodes.getLength(); ++i) {
			Node childNode = childNodes.item(i);
			if (childNode instanceof Element) {
				Element linkElement = (Element)childNode;
				printLinkElement(linkElement);
			}
		}
	}

	private static void printLinkElement(Element linkElement) {
		System.out.println("link " + linkElement.getAttribute("linkid") + ":" + linkElement.getAttribute("majorid") + " -> " + linkElement.getAttribute("minorid") + " (" + linkElement.getAttribute("link_type") + ")");
	}

	private static void printErrorElement(Element errorElement) {
		System.out.println("xml request error : " + errorElement.getFirstChild().getNodeValue());
		System.exit(1);
	}
	/** 
	 * Testing method. 
	 */
	public static void main(String[] args) {
		MySource mysource = MySource.getInstance();
		Document d = null;
		try {
			d = MySource.getInstance().doRequest("<command action=\"initialise\"/>");
			printDocument(d);
		} catch (IOException ioe) {
			System.err.println ("IO Exception : " + ioe.getMessage());
			System.exit(1);
		}
	}//end main()
}//end class
