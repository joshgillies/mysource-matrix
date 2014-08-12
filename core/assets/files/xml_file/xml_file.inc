<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: xml_file.inc,v 1.2 2012/08/30 01:09:07 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/files/file/file.inc';

/**
* XML_File
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Xml_File extends File
{


	/**
	* All allowed extensions for a xml
	*
	* @var Array(string)
	*/
	public $allowed_extensions = Array('xml');


	/**
	* Constructor
	*
	* @param int    $assetid    the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns the editable content of this asset
	*
	* @return string
	* @access public
	*/
	function getContent()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$parse_file = $this->data_path.'/'.$this->name;
		$content = file_to_string($parse_file);
	
		return trim($content);

	}//end getContent()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return void
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(),
				SQ_LINK_TYPE_2	=> Array(),
				SQ_LINK_TYPE_3	=> Array(),
				SQ_LINK_NOTICE	=> Array(
									'xsl_file'	=> Array(
													'card'		=> '1',
													'exclusive'	=> FALSE,
												   ),
									'image'		=> Array(
													'card'		=> '1',
													'exclusive'	=> FALSE,
												   ),
									'comment'	=> Array(
													'card'		=> '1',
													'exclusive'	=> FALSE,
												   ),
								   ),
			   );

	}//end _getAllowedLinks()


	public function printFrontend()
	{
		$xsl_file = NULL;
		$current = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'xsl_file', TRUE, 'transform');
		$current_id = array_get_index($current, 'minorid', 0);
		if (!empty($current_id)) {
			$xsl_file = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_id);
		}//end if

		if (!is_null($xsl_file) && class_exists('XSLTProcessor')) {
			Asset::printFrontend();	
		} else {
			parent::printFrontend();
		}//end if

	}//end printFrontend()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	public function printBody()
	{
		// Find the XSL Stylesheet
		$xsl_file = NULL;
		$current = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'xsl_file', TRUE, 'transform');
		$current_id = array_get_index($current, 'minorid', 0);
		if (!empty($current_id)) {
			$xsl_file = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_id);
		}//end if

		if (!is_null($xsl_file)) {
			// Do transformation
			$xml = new DOMDocument();
			$xml->loadXML($this->getContent());
			$xsl = new DOMDocument();

			$xsl_content = $xsl_file->getContent();
			if ($this->attr('xslt_file_replace_keywords')) {
				replace_global_keywords($xsl_content);
			}
			$xsl->loadXML($xsl_content);

			$parser = new XSLTProcessor();
			$parser->importStylesheet($xsl);

			$content = $parser->transformToXML($xml);
			echo $content;
		} else {
			echo $this->getContent();
		}//end if

	}//end printBody()


}//end class
?>
