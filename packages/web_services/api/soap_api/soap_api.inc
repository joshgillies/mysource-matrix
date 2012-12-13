<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_api.inc,v 1.13 2012/12/13 04:42:13 ewang Exp $
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
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_Api extends Asset
{

	public static $string_non_optional		= Array (
												'type'      => 'string',
						        				'min'		=> 1,
						        				'max'		=> 1,
						        				'nillable'	=> FALSE,
						        				'mixed'		=> FALSE,
											  );
	public static $string_optional			= Array (
												'type'      => 'string',
						        				'min'		=> 0,
						        				'max'		=> 1,
						        				'nillable'	=> FALSE,
						        				'mixed'		=> FALSE,
											  );
	public static $string_optional_unbounded= Array (
												'type'      => 'string',
						        				'min'		=> 0,
						        				'max'		=> 'unbounded',
						        				'nillable'	=> FALSE,
						        				'mixed'		=> FALSE,
											  );
	public static $boolean_non_optional		= Array (
												'type'      => 'boolean',
						        				'min'		=> 1,
						        				'max'		=> 1,
						        				'nillable'	=> FALSE,
						        				'mixed'		=> FALSE,
											  );
	public static $boolean_optional			= Array (
												'type'      => 'boolean',
						        				'min'		=> 0,
						        				'max'		=> 1,
						        				'nillable'	=> FALSE,
						        				'mixed'		=> FALSE,
											  );
	public static $int_non_optional			= Array (
												'type'      => 'int',
						        				'min'		=> 1,
						        				'max'		=> 1,
						        				'nillable'	=> FALSE,
						        				'mixed'		=> FALSE,
											  );
	public static $int_optional				= Array (
												'type'      => 'int',
						        				'min'		=> 0,
						        				'max'		=> 1,
						        				'nillable'	=> FALSE,
						        				'mixed'		=> FALSE,
											  );
	private $simple_types	= NULL;

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
		}//end if

		return TRUE;

	}//end _preCreateCheck()


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


    /**
     * This function will return an array about a particular input/output element for an operation
     *
     * @param $type
     * @param $min
     * @param $max
     * @param $nillable
     * @param $mixed
     * @return array
     * @access public
     */
	public static function getArgType($type, $min=0, $max=1, $nillable=FALSE, $mixed=FALSE)
	{
        $arg_type	= Array (
						'type'      => $type,
        				'min'		=> $min,
        				'max'		=> $max,
        				'nillable'	=> $nillable,
        				'mixed'		=> $mixed,
					  );
		return $arg_type;

	}//end getArgType()


	/**
	 * This function will return an array of all complex types for an API
	 *
	 * @param $complex_elements
	 *
	 * @return array
	 * @access public
	 */
	public static function getComplexElements($complex_elements=Array())
	{
        $complex_elements_available = Array();
        if (empty($func_list)) {
            // So sick, this is dodgy, it wouldnt hurt performance though.
            $func_list = $complex_elements;
        }//end if
            foreach ($func_list as $func_name => $switch) {
                if (isset($complex_elements[$func_name])) {
                    if ($switch) {
                        $complex_elements_available = array_merge($complex_elements_available, $complex_elements[$func_name]);
                    }//end if
                }//end if
            }//end foreach

        return $complex_elements_available;

	}//end getComplexElements()


	/**
	* This function take in a list of simpleTypes from each child asset and merge them
	*
	* @return array
	* @access public
	*/
	public static function processSimpleRestrictedTypes($simple_types=Array())
	{
        $simple_types_available = Array();
        if (empty($func_list)) {
            // So sick, this is dodgy, it wouldnt hurt performance though.
            $func_list = $simple_types;
        }//end if
            foreach ($func_list as $func_name => $switch) {
                if (isset($simple_types[$func_name])) {
                    if ($switch) {
                        $simple_types_available = array_merge($simple_types_available, $simple_types[$func_name]);
                    }//end if
                }//end if
            }//end foreach
		return $simple_types_available;

	}//end getSimpleRestrictedTypes()


	/**
	 * This function will return an array of all simple types for an API
	 *
	 * @param $simple_types
	 *
	 * @return array
	 * @access public
	 */
	public static function getSimpleRestrictedTypes($simple_types=Array())
	{
		return Array();

	}//end getSimpleRestrictedTypes()

	/**
	* getComplexTypes
	*
	* @return array
	* @access public
	*/
	public static function getComplexTypes($func_list=Array())
	{
		return Array();

	}//end getComplexTypes()


	/**
	 * This function try to get an index from an array and cast it to boolean
	 *
	 * @param $array array			The array we are trying to access the index from
	 * @param $index string			The index we are trying to get the data from
	 * @param $default_val	mixed	The default value
	 *
	 * @return boolean
	 */
	public static function getBooleanIndex($array, $index, $default_val)
	{
		$val	= array_get_index($array, $index, $default_val);
		if (empty($val)) {
			$val	= FALSE;
		} else if ($val == 1) {
			$val 	= TRUE;
		}//end else

		return $val;

	}//end getBooleanIndex()


	/**
	 * This function try to get an index from an array, if it is empty return NULL
	 *
	 * @param $array array			The array we are trying to access the index from
	 * @param $index string			The index we are trying to get the data from
	 * @param $default_val	mixed	The default value
	 *
	 * @return mixed
	 */
	public static function getNullIndex($array, $index, $default_val)
	{
		$val	= array_get_index($array, $index, $default_val);
		if (empty($val)) {
			$val	= NULL;
		}//end if
		return $val;

	}//end getNullIndex()


	/**
	 * This function try to get an index from an array, this function will restrict the value being enter to the declared restricted values
	 *
	 * @param $array array			The array we are trying to access the index from
	 * @param $index string			The index we are trying to get the data from
	 * @param $default_val	mixed	The default value
	 *
	 * @return mixed
	 */
	public function array_get_index_restricted_value($array, $index, $default_val)
	{
		$val	= array_get_index($array, $index, $default_val);
		if (empty($this->simple_types)) {
			$this->simple_types	= $this->getSimpleRestrictedTypes();
		}//end if

		if (!empty($this->simple_types) && isset($this->simple_types[$index])) {
			if (!in_array($val, $this->simple_types[$index]['enumeration'])) {
				throw new SoapFault('Server', 'The input value is not a valid value for '.$index.'. Valid values include: '.implode(', ', $this->simple_types[$index]['enumeration']));
			}//end if
		}//end if
		return $val;

	}//end array_get_index_restricted_value()


	
	/**
	* updates the lookup of the assetid passed and all its childrens, (runs freestyle HIPO)
	*
	* @return assetid
	* @access public
	*/
	public function updateLookupsforAsset($assetid)
	{
		require_once SQ_SYSTEM_ROOT.'/core/hipo/jobs/hipo_job_update_lookups.inc';
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

		$vars = Array('assetids' => Array($assetid));
		$hh->freestyleHipo('hipo_job_update_lookups', $vars);

	}//end updateLookupsforAsset()
	
}//end class
?>
