<?php
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
* $Id: metadata_field_multiple_text.inc,v 1.5 2009/10/16 01:39:28 lwright Exp $
*
*/


require_once dirname(__FILE__).'/../../metadata_field/metadata_field.inc';

/**
* Metadata_Field_Multiple_Text
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Multiple_Text extends Metadata_Field
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
	* Get an Option List attribute to print the values and edit interfaces
	*
	* @return Asset_Attribute_Option_List
	* @access public
	*/
	public function getOptionListAttribute()
	{
		require_once SQ_ATTRIBUTES_PATH.'/option_list/option_list.inc';

		$attribute = new Asset_Attribute_Option_List();

		if ((boolean)$this->attr('is_contextable') === TRUE) {
			$default = implode($attribute->delimiter, explode('; ', $this->attr('default')));
		} else {
			$default = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('default', 'metadata_field', Array($this->id), 0);
			$default = array_get_index($default, $this->id, '');
		}

		$default = implode($attribute->delimiter, explode('; ', $default));

		if ($attribute->validateValue($default)) {
			$attribute->setValue($default);
		}

		return $attribute;

	}//end getOptionListAttribute()


}//end class
?>