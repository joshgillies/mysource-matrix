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
* $Id: metadata_field_date.inc,v 1.12.4.1 2006/12/27 23:01:27 tbarrett Exp $
*
*/

require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
require_once dirname(__FILE__).'/../../metadata_field/metadata_field.inc';

/**
* Metadata_Field
*
* Purpose
*
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Revision: 1.12.4.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Date extends Metadata_Field
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Metadata_Field_Date($assetid=0)
	{
		$this->Metadata_Field($assetid);

	}//end constructor


	/**
	* Generate an array of current values for the passed metadata field names
	*
	* @param object	&$asset		the asset whose field values we are getting
	* @param string	$value_str	the current value for this field in the asset (NOT $asset)
	*							NULL means that there is no current value set
	*
	* @return string
	* @access public
	*/
	function getMetadataValue(&$asset, $value_str)
	{
		$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		if (is_null($value_str)) {
			$value = $this->attr('default');
			$value_components = $this->attr('value_components');
		} else {
			$value='';
			$value_components = $this->attr('value_components');
			Metadata_Field::decodeValueString($value_str, $value, $value_components);
		}

		if (substr($value, 0, 1) == '%') {

			// strip off the last % sign from the end of $key string
			$key = substr($value, 1, -1);

			// prepare date format for merging - we will replace all spaces with underscores
			$format = str_replace(' ', '~', $this->attr('format'));

			// merge the values with '_', add closing % sign and proceed...
			$keyword = $key.'_'.$format;
			$offset = $this->attr('offset');
			if (!empty($offset)) {
				$offset = str_replace(' ', '', $offset);
				$keyword .= '_'.$offset;
			}

			$metadata_values = $mm->generateKeywordReplacements($asset, Array($keyword), FALSE);
			$value = $metadata_values[$keyword];

		} else {
			$datetime_field = $this->_getDateTimeField('', $value);
			$value = $datetime_field->format();
		}

		return Metadata_Field::encodeValueString($value, $value_components);

	}//end getMetadataValue()


	/**
	* Getting the date-time field
	*
	* @param string	$prefix	prefix for form elements
	* @param string	$value	the value to place in the datetime field
	*
	* @return object
	* @access private
	*/
	function _getDateTimeField($prefix, $value=NULL)
	{
		// parameters
		$parameters = $this->attr('setup_params');
		if (empty($parameters)) {
			$date = getdate();
			$parameters = Array(
							'min'			=> '2003-01-01 00:00:00',
							'max'			=> ($date['year'] + 1).'-12-31 23:59:59',
							'allow_circa'	=> '0',
							'show'			=> Array('y', 'm', 'd', 'h', 'i', 's'),
							'null'			=> Array(),
							'style'			=> Array(
												'y'	=> 's',
												'm'	=> 's',
												'd'	=> 's',
												'h'	=> 's',
												'i'	=> 's',
												's'	=> 's',
											   ),
						  );
		}
		$parameters['print_format'] = $this->attr('format');

		// only if we are printing the default value
		if (is_null($value)) {
			$value = $this->attr('default');
			if (!empty($value) && $value{0} == '%') {
				$value = ts_iso8601(time());
			} else {
				$value = $parameters['min'];
			}
		}

		$field =& new DateTime_Field($prefix.'_datetime', $value, $parameters);

		return $field;

	}//end _getDateTimeField()


}//end class

?>
