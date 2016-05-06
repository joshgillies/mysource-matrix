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
* $Id: metadata_field_related_asset.inc,v 1.2 2012/08/30 01:09:09 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../metadata_field/metadata_field.inc';

/**
* Metadata Field Related Asset
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Related_Asset extends Metadata_Field
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor

	
	/**
	* Takes a metadata value and value-components and returns a properly escaped string
	* Since "value components" is not applicable to this metadata field, we don't need to encode/decode the value string
	* 
	* @return void
	* @access public
	* @see decodeValueString()
	*/
	public static function encodeValueString($value, $components)
	{
		return $value;

	}//end encodeValueString()


	/**
	* Takes a full metadata string (i.e. value and value components) and returns the value and value components
	* Since "value components" is not applicable to this metadata field, we don't need to encode/decode the value string
	* 
	* @return void
	* @access public
	* @see encodeValueString()
	*/
	public static function decodeValueString($str, &$value, &$components)
	{
		$value = $str;
		$components = Array();

	}//end decodeValueString()

}//end class

?>
