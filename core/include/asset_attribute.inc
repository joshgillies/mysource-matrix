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
* $Id: asset_attribute.inc,v 1.58 2012/08/30 01:09:19 ewang Exp $
*
*/


/**
* Asset Attribute
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.58 $
* @package MySource_Matrix
*/
class Asset_Attribute extends MySource_Object
{

	/**
	* The unique ID for this attribute
	* @var int
	*/
	var $id = 0;

	/**
	* The name of this attribute
	* @var string
	*/
	var $name = '';

	/**
	* Whether this attribute value is unique
	* @var boolean
	*/
	var $uniq = FALSE;

	/**
	* The description of this attribute
	* @var string
	*/
	var $description = '';

	/**
	* Parameters for processing/calculating/validating this attribute
	* @var Array()
	*/
	var $_params = Array();

	/**
	* The default value for this attribute
	* @var mixed
	*/
	var $_default_value = NULL;

	/**
	* Parameters for displaying this attribute
	* @var Array()
	*/
	var $_edit_params = Array();

	/**
	* The current value for this instance of this attribute
	* @var mixed
	*/
	var $value = NULL;

	/**
	* Whether or not this attribute has processed the results of a form post successfully
	* @var mixed
	*/
	var $processed = FALSE;

	/**
	* Whether or not this attribute is used for administrative purposes only
	* @var boolean
	*/
	var $is_admin = FALSE;


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	* @param array	$default_params	the default values for the params
	*
	*/
	function Asset_Attribute($attributeid=0, $value=NULL, $default_params=Array())
	{
		$this->MySource_Object();

		if ($attributeid) {
			$db = DAL::getDb();

			$sql = 'SELECT attrid, name, type, uniq, description, is_admin, parameters_val, default_val
					FROM sq_ast_attr
					WHERE attrid = :attrid';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'attrid', $attributeid);
				$result = DAL::executePdoAll($query);
				if (empty($result)) {
					return;
				} else {
					$row = $result[0];
				}
				unset($result);
			} catch (Exception $e) {
				throw new Exception('Unable to get information for attribute ID '.$attributeid.' due to database error: '.$e->getMessage());
			}

			$this->id             = $row['attrid'];
			$this->name           = $row['name'];
			$type                 = $row['type'];
			$this->uniq           = $row['uniq'];
			$this->description    = $row['description'];
			$this->is_admin       = $row['is_admin'];
			$parameters           = $row['parameters_val'];
			$this->_default_value = $row['default_val'];

			// make sure the asset we are loading is of the same type as our class
			if ($type != $this->type()) {
				throw new Exception('Attribute type mismatch. Attribute ID '.$attributeid.' was created as a "'.$this->type().'" attribute but the database believes it is a "'.$type.'" attribute.');
				//$this->id = NULL;
				return;
			}

			$this->_params = ($parameters) ? unserialize($parameters) : Array();
			unset($result);

		}

		$this->value = $value;

		// now just make sure some default values are set
		foreach ($default_params as $name => $default) {
			if (!isset($this->_params[$name])) {
				$this->_params[$name] = $default;
			}
		}

	}//end constructor


	/**
	*
	* Retrieves subclasses of Asset_Attribute, static method
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	* @param array	$default_params	the default values for the params
	*
	* @return mixed object
	* @access public
	*/
	function &loadAttribute($attributeid=0, $value=NULL, $default_params=Array())
	{
		if ($attributeid == 0) {
			$empty_attr = new Asset_Attribute(0, $value, $default_params);
			return $empty_attr;
		}

		$db = DAL::getDb();

		$sql = 'SELECT type FROM sq_ast_attr
					WHERE attrid = :attrid';
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'attrid', $attributeid);
			$type = DAL::executePdoOne($query);
		} catch (Exception $e) {
			throw new Exception('Unable to load attribute ID #'.$attributeid.' due to database error: '.$e->getMessage());
		}

		$class = 'Asset_Attribute_'.$type;
		require_once SQ_ATTRIBUTES_PATH.'/'.$type.'/'.$type.'.inc';
		$attr = new $class($attributeid, $value, $default_params);

		return $attr;

	}//end loadAttribute()


	/**
	* Returns the type of this attribute
	*
	* @return string
	* @access public
	*/
	function type()
	{
		// strip the 'Asset_Attribute_' off the front
		// And lower-case it - PHP 5 uses the case the class was declared as
		return strtolower(substr(get_class($this), 16));

	}//end type()


	/**
	* Prints the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form element
	* @param boolean	$read_only	are we just printing the value
	*
	* @return void
	* @access public
	*/
	function paint($prefix, $read_only=FALSE)
	{
		echo translate('interface_not_created', ucwords(str_replace('_', ' ', $this->type())));

	}//end paint()


	/**
	* Print the interface for changing some params
	*
	* Attributes must override this function
	*
	* @param string		$prefix			prefix for the form element
	* @param boolean	$write_access	write access
	*
	* @return boolean
	* @access public
	*/
	function paintEditParams($prefix, $write_access=FALSE)
	{
		return TRUE;

	}//end paintEditParams()


	/**
	* Process the interface for changing some params
	*
	* Attributes must override this function
	*
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processEditParams($prefix)
	{
		return TRUE;

	}//end processEditParams()


	/**
	* Prints the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	function process($prefix)
	{
		$this->processed = FALSE;

	}//end process()


	/**
	* Validate the value of this attribute - children should overwrite
	*
	* This function should return FALSE if the passed-in value is irreparably invalid.  If the value
	* is only trivially invalid this function may tweak it to make it valid, but it is NOT the job of
	* this function to change an invalid value to this attribute's default value.
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		return TRUE;

	}//end validateValue()


	/**
	* Sets the value of this attribute after validating
	*
	* @param mixed	 &$value	value to be validated
	* @param boolean $validate	Whether to validate the value
	*
	* @return boolean
	* @access public
	*/
	function setValue(&$value, $validate=TRUE)
	{
		if ($validate && !$this->validateValue($value)) {
			trigger_localised_error('SYS0073', E_USER_WARNING, $value, $this->type());
			return FALSE;
		}
		$this->value = $value;
		return TRUE;

	}//end setValue()


	/**
	* Takes an XML_Tree_Node from an edit interface XML file and creates
	* the params array from it
	*
	* @param object	&$node	SimpleXMLElement
	*
	* @return boolean
	* @access public
	*/
	function setEditParams(&$node)
	{

		if (strtolower($node->getName()) != strtolower($this->type())) {
			trigger_localised_error('SYS0300', E_USER_WARNING, $node->getName(), $this->type(), '<pre>'.htmlspecialchars($node->asXml()).'</pre>');
			return FALSE;
		}
		return TRUE;

	}//end setEditParams()


	/**
	* Manually set the value of an edit parameter for this attribute
	*
	* The parameter must already contain a default value to be set with this function
	* It returns TRUE if parameter set successfully
	*
	* @param string	$field	The parameter field to set
	* @param mixed	$value	The value to set it to
	*
	* @return boolean
	* @access public
	*/
	function setEditParam($field, $value)
	{
		if (isset($this->_edit_params[$field])) {
			$this->_edit_params[$field] = $value;
			return TRUE;
		} else {
			trigger_localised_error('SYS0258', E_USER_NOTICE, $field, $this->type());
			return FALSE;
		}

	}//end setEditParam()


	/**
	* Manually set the value of an  parameter for this attribute
	*
	* The parameter must already contain a default value to be set with this function
	* It returns TRUE if parameter set successfully
	*
	* @param string	$field	The parameter field to set
	* @param mixed	$value	The value to set it to
	*
	* @return boolean
	* @access public
	*/
	function setParam($field, $value)
	{
		if (isset($this->_params[$field])) {
			$this->_params[$field] = $value;
			return TRUE;
		} else {
			trigger_localised_error('SYS0258', E_USER_NOTICE, $field, $this->type());
			return FALSE;
		}

	}//end setParam()


	/**
	* Returns the raw content of this attribute type
	*
	* @return string
	* @access public
	*/
	function getContent()
	{
		return $this->value;

	}//end getContent()


	/**
	* Returns the content of this attribute as it should be shown in a keyword replacement
	*
	* @param string	$format	Format suffix
	*
	* @return void
	* @access public
	*/
	function getKeywordValue($format='')
	{
		if ($format !== '') return ''; // no formats supported by default
		return $this->value;

	}//end getKeywordValue()


	/**
	* Returns a representation of this asset in XML
	*
	* @return string
	* @access public
	*/
	function exportToXML()
	{
		$xml='';

		$tag_attributes  = 'name="'.$this->name.'" ';
		$tag_attributes .= 'id="'.$this->id.'" ';
		$tag_attributes .= 'uniq="'.$this->uniq.'" ';
		$tag_attributes .= 'description="'.$this->description.'" ';
		$tag_attributes .= 'default_value="'.$this->_default_value.'" ';
		$tag_attributes .= 'processed="'.$this->processed.'" ';
		$tag_attributes .= 'is_admin="'.$this->is_admin.'" ';

		$open_tag = '<'.$this->type().' '.$tag_attributes.'>';
		$close_tag = '</'.$this->type().'>';

		$contents = $this->exportContentsToXML();

		$xml = $open_tag.$contents.$close_tag;

		return $xml;

	}//end exportToXML()


	/**
	* Returns a representation of this asset's parameters, edit parameters and value as XML
	*
	* This function will be overridden by children who do not use getContent to get their value
	*
	* @return string
	* @access public
	*/
	function exportContentsToXML()
	{
		$xml = '<value><![CDATA['.$this->getContent().']]></value>';
		return $xml;

	}//end exportContentsToXML()


}//end class

?>
