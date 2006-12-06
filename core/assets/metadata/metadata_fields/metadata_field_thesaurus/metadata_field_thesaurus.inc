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
* $Id: metadata_field_thesaurus.inc,v 1.3 2006/12/06 01:07:36 emcdonald Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/metadata/metadata_field/metadata_field.inc';

/**
* Metadata_Field_Select
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Thesaurus extends Metadata_Field
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Metadata_Field_Thesaurus($assetid=0)
	{
		$this->Metadata_Field($assetid);

	}//end constructor


	/**
	* Returns a reference to the parent thesaurus asset
	*
	* @return object
	* @access public
	*/
	function &getThesaurusRef()
	{
		$th_id = $this->attr('thesaurus_assetid');
		$false = FALSE;
		if (empty($th_id)) return $false;

		return $GLOBALS['SQ_SYSTEM']->am->getAsset($th_id);

	}//end getThesaurusRef()


}//end class

?>
