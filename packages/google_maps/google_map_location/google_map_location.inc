<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: google_map_location.inc,v 1.5 2012/08/30 00:58:46 ewang Exp $
*
*/





require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Google Map Location
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage google_maps
*/
class Google_Map_Location extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid		assetid for the asset
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

    }//end constructor


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$attr = ($short_name) ? 'short_name' : 'name';
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName($attr, $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


    /**
    * Perform any pre-validation required during the creation of this asset
    *
    * Pages ensure a name and short_name has been set before they are created
    *
    * @param array  &$link  information used to create the initial link
    *
    * @return boolean
    * @access private
    */
    function _preCreateCheck(&$link)
    {
        if (!parent::_preCreateCheck($link)) return FALSE;

        $name = trim($this->attr('name'));
        if ($name == '') {
            trigger_localised_error('CORE0083', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
            return FALSE;
        }

        // if the short name hasn't been set then short and long names are the same to begin with
        if (trim($this->attr('short_name')) == '') {
            $this->setAttrValue('short_name', $name);
        }

        return TRUE;

    }//end _preCreateCheck()


}//end class
?>
