<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: page.inc,v 1.27 2003/10/21 06:25:43 dwong Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Page
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page extends Asset
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page($assetid=0)
	{
		$this->Asset($assetid);
	}


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$name = trim($this->attr('name'));

		if ($name == '') {
			trigger_error('Unable to create a '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' with a blank name', E_USER_WARNING);
			return false;
		}

		// short and long names are the same to begin with
		$this->setAttrValue('short_name', $name);

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);
		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		// set an initial web path
		$initial_path = strtolower($name);
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $this->id, true);
		if (!$this->saveWebPaths($good_paths)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	}//end create()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$allowed = parent::_getAllowedLinks();
		$allowed[SQ_LINK_TYPE_1]['page'] = Array('card' => 'M', 'exclusive' => false);
		$allowed[SQ_LINK_TYPE_1]['file'] = Array('card' => 'M', 'exclusive' => false);

		$allowed[SQ_LINK_TYPE_2]['page'] = Array('card' => 'M', 'exclusive' => false);
		$allowed[SQ_LINK_TYPE_2]['file'] = Array('card' => 'M', 'exclusive' => false);

		$allowed[SQ_LINK_TYPE_3]['page'] = Array('card' => 'M', 'exclusive' => false);

		return $allowed;

	}//end _getAllowedLinks()


	/**
	* Called by the design to print the body of this asset
	* it's up to the kids to override
	*
	* @access public
	*/
	function printBody()
	{
	}// end printBody()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return (($short_name) ? $this->attr('short_name') : $this->attr('name'));

	}//end _getName();


	/** 
	* Retrieves a list of key data from this asset (used by the indexing service).
	* Subclasses override this to give the content they want to make searchable.
	* 
	* @access public
	* @param mixed $component If it is not false, then it is assumed to be the name of a component of this asset (defined and controlled by subclasses).
	* @return array An array of all the keydata for this asset. Keydata is of the form
	*				array (
	*					array (
	*						'value' => ...,
	*						'type' => ..., (text, date, number, etc.)
	*						'component' => ..., (whatever you want, just as long as this asset knows how to deal with it if it needs to)
	*					), ...
	*				)
	*/
	function &getKeyData($component = false)
	{
		$keyData = parent::getKeyData($component);

		$weightings = $this->getWeightings();

		if (!$component || $component == 'short_name') {
			$short_name_attribute	=& $this->getAttribute('short_name');
			$value = $short_name_attribute->value;
			foreach (get_word_counts($value) as $word => $count) {
				$this->buildKeyData($keyData, $word, 'text', "short_name", $count * $weightings['short_name']['weighting']);
			}
		}

		return $keyData;
	} // end getKeyData()

}#end class
?>
