<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Constant_Button
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Constant_Button extends Design_Area
{

	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		// if they are trying to set the type_codes, split the string
		if ($name == 'type_codes' && !is_array($value)) {
			$value = preg_split('/[ ,]+/', (string) $value);

		}// end if

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/*
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @return boolean
	* @access public
	*/
	function printArea(&$design) 
	{
		$assetid = (int) $this->attr('assetid');
		$url     = $this->attr('url');

		// return true because it just means that they haven't set anything yet
		if (empty($assetid) && empty($url)) return true;

		// if there is no assetid then just send the url along the way
		if (empty($assetid)) {
			$url_code = $url;

		} else {
			$url_code = '<'.'?php echo $constant_btn_'.$this->attr('id_name').'_url; ?'.'>';
			echo '<', '?php
			$constant_btn_'.$this->attr('id_name').'_asset = &$GLOBALS[\'SQ_SYSTEM\']->am->getAsset('.$assetid.');
			if (is_null($constant_btn_'.$this->attr('id_name').'_asset)) {
				$constant_btn_'.$this->attr('id_name').'_url = \'\';
			} else {
				$constant_btn_'.$this->attr('id_name').'_url = $constant_btn_'.$this->attr('id_name').'_asset->getHref();
			}// end if
			?', '>';
		}// end if

		$class    = ($this->attr('class')) ? 'class="'.$this->attr('class').'"' : '';
		$a_tag    = $class;
		$contents = '';

		// if we are poping up in a new window, adjust the vars accordingly
		if ($this->attr('new_window')) {
			$width  = (int) $this->attr('new_window_width');
			$height = (int) $this->attr('new_window_height');
			 // some defaults just in case
			if ($width  <= 0) $width  = 640;
			if ($height <= 0) $height = 600;
			$a_tag .= ' onClick="Javascript: new_window = window.open(\''.$url_code.'\', \'constant_btn_'.$this->attr('id_name').'\', \'toolbar=no,width='.$width.',height='.$height.',titlebar=false,scrollbars=yes\');new_window.focus(); return false;" ';
			$url_code = '#';

		// if we have a target set it up
		} else if ($this->attr('target')) {
			$a_tag .= ' target="'.$this->attr('target').'" ';

		}// end if

		// if there is an image then use it
		if ($image = $this->attr('image')) {

			$name = 'constant_btn_'.$this->attr('id_name');

			$data_suffix = $this->type().'/'.$this->id;

			$mouseover  = '';
			$mouseout   = '';

			// if this is a rollover 
			if ($image_rollover = $this->attr('image_rollover')) { 
				$a_tag .= ' onMouseOver="javascript: imgRoll("'.$name.'"); return true;" onMouseOut="javascript: imgRoll("'.$name.'"); return true;"';
				echo '<script language="javascript" type="text/javascript"> AddRollover("'.$name.'", "<'.'?php echo $DATA_PATH;?'.'>/'.$data_suffix.'/'.$image_rollover.'"); </script>';
			}//end if

			// if there is a current button and we are the current page
			if (!empty($assetid) && ($image_current = $this->attr('image_current'))) { 
				$image = '<'.'?php echo (($ASSET->id == '.$assetid.') ? '.var_export($image_current, true).' : '.var_export($image, true).'; ?'.'>';
			}//end if

			$contents = '<img src="<'.'?php echo $DATA_PATH; ?'.'>/'.$data_suffix.'/'.$image.'" '.$class.' name="'.$name.'" id="'.$name.'" border="0" '.$this->attr('extra').'>';

		// else there is text
		} else {
			$contents = $this->attr('text');
			if (empty($contents)) $contents = $url_code;

		}//end if

		echo '<a href="', $url_code, '" ', $a_tag, '>', $contents, '</a>';

		return true;

	}//end printArea()

}//end class

?>
