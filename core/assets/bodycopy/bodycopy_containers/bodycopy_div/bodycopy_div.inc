<?php
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
* $Id: bodycopy_div.inc,v 1.12 2005/07/15 04:28:59 tbarrett Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/bodycopy/bodycopy_container/bodycopy_container.inc';

/**
* Bodycopy_Div
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Bodycopy_Div extends Bodycopy_Container
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Bodycopy_Div($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Bodycopy_Container($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Bodycopy DIVs create a WYSIWYG content type when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @access private
	* @return boolean
	*/
	function _createAdditional(&$link)
	{
		// grab the bodycopy data that we stored in the create() function
		$bc_data = $this->_tmp['bodycopy_create_data'];
		unset($this->_tmp['bodycopy_create_data']);

		if (!parent::_createAdditional($link)) return false;

		// set the name to what they entered - if they entered anything, otherwise use a default unique name
		if (isset($bc_data['attributes']['identifier']) && trim($bc_data['attributes']['identifier']) != '') {
			$this->setAttrValue('name', $bc_data['attributes']['identifier']);
		} else {
			$this->setAttrValue('name', $this->attr('name').' '.$this->id);
		}

		// create a content type
		$content_type = $GLOBALS['SQ_SYSTEM']->getUserPrefs('bodycopy_container', 'SQ_DEFAULT_CONTENT_TYPE');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($content_type);
		$wysiwyg =& new $content_type();
		$div_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'value' => 'div_contents', 'sort_order' => 1, 'is_dependant' => 1, 'is_exclusive' => 1);
		$wysiwyg->setAttrValue('name', 'DIV Content');
		if (isset($bc_data['content']) && (trim($bc_data['content'] != ''))) {
			$wysiwyg->setAttrValue('html', $bc_data['content']);
		}
		if (!$wysiwyg->create($div_link)) return false;

		if (!($lock_status = $GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes'))) {
			return false;
		}

		$edit_fns = $this->getEditFns();
		if (!$this->saveAttributes() || !$edit_fns->generateContentFile($this)) {
			return false;
		}

		if ($lock_status == 1) {
			if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'atttributes')) {
				return false;
			}
		}

		return true;

	}//end _createAdditional()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_2 => Array('content_type' => Array('card' => '1', 'exclusive' => true)),
					 SQ_LINK_NOTICE => Array('asset' => Array('card' => 'M', 'exclusive' => false))
					 );

	}//end _getAllowedLinks()


}//end class

?>