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
* $Id: metadata_field_date.inc,v 1.21 2012/08/30 01:09:08 ewang Exp $
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
* @version $Revision: 1.21 $
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
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Generate an array of current values for the passed metadata field names
	*
	* @param Asset	$asset		the asset whose field values we are getting
	* @param string	$value_str	the current value for this field in the asset (NOT $asset)
	*							NULL means that there is no current value set
	*
	* @return string
	* @access public
	*/
	public function getMetadataValue(Asset $asset, $value_str)
	{
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

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
	public function _getDateTimeField($prefix, $value=NULL)
	{
		// parameters
		$parameters = $this->attr('setup_params');
		if (empty($parameters)) {
			$date = getdate();
			$parameters = Array(
							'min'			=> $date['year'].'-01-01 00:00:00',
							'max'			=> ($date['year'] + 10).'-12-31 23:59:59',
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
			if ((boolean)$this->attr('is_contextable') === TRUE) {
				$value = $this->attr('default');
			} else {
				$value = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('default', 'metadata_field', Array($this->id), 0);
				$value = $value[$this->id];
			}

			if (!empty($value) && $value{0} == '%') {
				$value = ts_iso8601(time());
			} else {
				$value = $parameters['min'];
			}
		}

		$field = new DateTime_Field($prefix.'_datetime', $value, $parameters);

		return $field;

	}//end _getDateTimeField()


}//end class
?>