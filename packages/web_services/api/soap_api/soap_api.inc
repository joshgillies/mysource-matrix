<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_api.inc,v 1.1.2.1 2008/05/12 04:54:07 hnguyen Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Soap API
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.1.2.1 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_Api extends Asset
{


    /**
    * Constructor
    *
    * @param int    $assetid    the asset id to be loaded
    *
    */
    function __construct($assetid=0)
    {
		$this->_ser_attrs = TRUE;
        parent::__construct($assetid);

    }//end constructor


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Pages ensure a name and short_name has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _preCreateCheck(Array &$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		// if the short name hasn't been set then short and long names are the same to begin with
		$name = trim($this->attr('name'));
		if (trim($this->attr('short_name')) == '') {
			$this->setAttrValue('short_name', $name);
		}

		return TRUE;

	}//end _preCreateCheck()


    /**
    * Returns name of the asset
    *
    * @param boolean    $short_name whether or not we are after the short name or the full name
    *
    * @return string
    * @access private
    * @see Asset::_getName()
    */
    protected function _getName($short_name=FALSE)
    {
        return (($short_name) ? $this->attr('short_name') : $this->attr('name'));

    }//end _getName()


    /**
    * Returns a function list of the asset let the children handle this
    *
    * @return string
    * @access private
    * @see Asset::_getName()
    */
    public function getFunctionList()
    {
        return Array();

    }//end getFunctionList()


	public static function getArgType($type, $optional=FALSE)
	{
        $arg_type	= Array (
						'type'      => $type,
						'optional'  => $optional,
					  );
		return $arg_type;

	}//end getArgType()


	public static function getComplexTypes($complex_types=Array())
	{
        $complex_types_available = Array();
        if (empty($func_list)) {
            // So sick, this is dodgy, it wouldnt hurt performance though.
            $func_list = $complex_types;
        }//end if
            foreach ($func_list as $func_name => $switch) {
                if (isset($complex_types[$func_name])) {
                    if ($switch) {
                        $complex_types_available = array_merge($complex_types_available, $complex_types[$func_name]);
                    }//end if
                }//end if
            }//end foreach


        return $complex_types_available;

	}//end getComplexTypes()

}//end class
?>
