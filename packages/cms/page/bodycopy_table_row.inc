<?  ##############################################
   ### MySource ------------------------------###
  ##- Bodycopy Editor ---- PHP4 --------------##
 #-- Copyright Squiz.net ---------------------#
##############################################
## This file is subject to version 1.0 of the
## MySource License, that is bundled with
## this package in the file LICENSE, and is
## available at through the world-wide-web at
## http://mysource.squiz.net/
## If you did not receive a copy of the MySource
## license and are unable to obtain it through
## the world-wide-web, please contact us at
## mysource@squiz.net so we can mail you a copy
## immediately.
##
## Desc: A class that handles tables rows within tables of the body copy
## $Source: /home/csmith/conversion/cvs/mysource_matrix/core/mysource_matrix/packages/cms/page/Attic/bodycopy_table_row.inc,v $
## $Revision: 1.1 $
## $Author: gsherwood $
## $Date: 2003/05/08 06:25:50 $
#######################################################################
include_once(dirname(__FILE__).'/bodycopy_table_cell.inc');
#---------------------------------------------------------------------#


/** 
* @access public
* @package Bodycopy
*/
class BodyCopy_Table_Row extends Object {

	var $attributes = Array();  # an array of attributes for the TR tag
	var $cells      = Array();  # an array of table cells (BodyCopy_Table_Cell)
	

	 ##############################
	# Constructor
	function BodyCopy_Table_Row($num_cols=1, $default_cell_type='') {

		# just in case :P
		$num_cols = intval($num_cols);

		for($i = 0; $i < $num_cols; $i++) {
			$this->cells[$i] = new BodyCopy_Table_Cell($default_cell_type);
		}#end for

	}#end constructor

	 #########################################
	# set an attribute for the table tag
	function set_attribute($name, $value) {
		$name = strtolower($name);
		if ($this->attributes[$name] == $value) return '';
		$this->attributes[$name] = $value;
		return $name.' changed to \''.$value.'\'';
	}

	 ##############################
	# delete this row
	function delete() {

		unset($this->attributes);

		# run through the cells and delete() them
		for($i = 0; $i < count($this->cells); $i++) {
			$this->cells[$i]->delete();
		}#end for
		unset($this->cells);

		$this = new BodyCopy_Table_Row();
		return true;

	}#end delete()


	 #########################################################
	# insert a new table cell before or after the passed cellid
	function insert_cell($cellid, $before=true, $default_cell_type='', $colspan=false) {

		$cellid = intval($cellid);

		if (!$colspan) {

			# check for colspan's in this row - colspan's will muck things up
			$colspan_error = 0;
			$colspaning = 0;
			for($i = 0; $i < count($this->cells); $i++) {
				if ($this->cells[$i]->colspan() > 1) { 
					$colspan_error = 1;
				}
			}
			if ($colspan_error == 1) {
				# work out the correct cellid for the id passed in
				$real_cellid = 0;
				for($i = 0; $i < count($this->cells); $i++) {
					if ($colspaning >= $cellid) { break; }
					$colspaning += intval($this->cells[$i]->colspan());
					if (intval($this->cells[$i]->colspan()) == 0) $colspaning++;
					if ($colspaning <= $cellid) {
						# havn't found the cell yet
						$real_cellid++;
					}
				}

				
				# lets try and find out how many cells are before the cell we are looking at
				$colspaning = 0;
				for($i = 0; $i < $real_cellid; $i++) {
					$colspaning += intval($this->cells[$i]->colspan());
					if (intval($this->cells[$i]->colspan()) == 0) $colspaning++;
				}

				if ($real_cellid < 0) $real_cellid = 0;

				# now we have the real cellid, so we will be increasing the colspan of this cell
				# (if it has one) unless we are supposed to be inserting AFTER this cell or BEFORE
				# this cell
				# so lets check shall we?

				$dont_do_this = false;

				# if the real cellid is found to be the same as the cellid, we have a special
				# situation where we have found the first cell in a colspan that has no
				# colspans before it on the row
				if ($cellid == $real_cellid) {
					if ($before) $dont_do_this = true;
				} else {
					if ((($this->cells[$real_cellid]->colspan() + $colspaning) == ($cellid + 1)) && !$before) $dont_do_this = true;
				}

				if ($cellid == 0 && $before) $dont_do_this = true;
					
				$cellid = $real_cellid;
				if ($cellid < 0) $cellid = 0;

				# if this cell has colspan's already, we can just increase the colspan by 1
				if ($this->cells[$cellid]->colspan() > 1 && !$dont_do_this) {
					$colspaning = $this->cells[$cellid]->colspan() + 1;
					$this->cells[$cellid]->set_attribute('colspan', $colspaning);
					return 'Colspan of cell '.($cellid + 1).' has been increased';
				}
				
			}
		}

		# if the cell is greater than the number of cells we have set it to the last on
		if ($cellid > count($this->cells) - 1) $cellid = count($this->cells) - 1;
		if ($cellid < 0) $cellid = 0;
		$compareid = $cellid;

		# if they want to insert after the passed cell what 
		# they really mean is to insert before the next cell
		if (!$before) $cellid++;

		# move all the cells up one position
		for($i = count($this->cells); $i > $cellid; $i--) {
			$this->cells[$i] = $this->cells[$i - 1];
		}#end for

		if ($cellid <= $compareid) $compareid++;

		# now add the cell
		$this->cells[$cellid] = new BodyCopy_Table_Cell($default_cell_type);

		# now that the cell exists, set its properties to
		# the properties of the cell to the left/right
		foreach ($this->cells[$compareid]->attributes as $name => $val) {
			if (strtolower($name) == 'colspan') continue;
			$this->cells[$cellid]->set_attribute($name,$val);
		}
		$this->cells[$cellid]->set_type(get_class($this->cells[$compareid]->type));
       


		return 'Cell inserted '.(($before) ? 'before' : 'after').' cell '.(($before) ? $cellid + 1 : $cellid);

	}#end insert_cell()

	 #########################################################
	# delete a the passed table cell from the array
	function delete_cell($cellid, $colspan=false) {

		if (!$colspan) {

			# check for colspan's in this row - colspan's will muck things up
			$colspan_error = 0;
			$colspaning = 0;
			for($i = 0; $i < count($this->cells); $i++) {
				if ($this->cells[$i]->attributes['colspan'] > 1) { 
					$colspan_error = 1; 
				}
			}
			if ($colspan_error == 1) {
				# work out the correct cellid for the id passed in	
				$real_cellid = 0;
				for($i = 0; $i < count($this->cells); $i++) {						
					if ($colspaning >= $cellid) { break; }
					$colspaning += intval($this->cells[$i]->attributes['colspan']);
					if (intval($this->cells[$i]->attributes['colspan']) == 0) { $colspaning++; }
					if ($colspaning <= $cellid) {
						# havn't found the cell yet
						$real_cellid++;
					}
				}

				$cellid = $real_cellid;

				# if this cell has colspan's already, we can just reduce the colspan by 1
				if ($this->cells[$cellid]->attributes['colspan'] > 1) {
					$colspaning = $this->cells[$cellid]->attributes['colspan'] - 1;
					$this->cells[$cellid]->set_attribute('colspan', $colspaning);
					return 'Colspan of cell '.($cellid + 1).' has been reduced to '.$colspaning;
				}
			}
		}

		# make sure cell exists
		if ($this->cells[$cellid] && count($this->cells) > 1) {
			# do any clean up
			$this->cells[$cellid]->delete();

			# move all the cells down one position
			for($i = $cellid; $i < count($this->cells) - 1; $i++) {
				$this->cells[$i] = $this->cells[$i + 1];
			}#end for

			# remove the last element
			array_pop($this->cells);
			return 'Cell '.($cellid + 1).' has been deleted';

		} elseif (count($this->cells) == 1) {
			return 'Cell '.($cellid + 1).' cannot be deleted as it is the only cell left';

		}#end if

		return '';

	}#end delete_cell()

	 #########################################################
	# re-orders the row cells, takes an array of the structure (new_place => old_place)
	function reorder_cells($cell_order) {

		# if there ain't no changes, die
		$changes = false;
		foreach($cell_order as $new_place => $old_place) $changes |= ($new_place != $old_place);
		if (!$changes) return '';

		# if there is a conflict in the number of cells, die
		if (count($this->cells) != count(array_unique($cell_order))) return '';

		$new_array = Array();
		for($i = 0; $i < count($cell_order); $i++) {
			$new_array[$i] = $this->cells[$cell_order[$i]];
		}
		# now assign the new ordered array
		$this->cells = $new_array;

		return 'Cell\'s have been reordered';

	}#end reorder_cells()

	 ########################
	# paints the table row 
	function paint(&$keyword_replacements) {

		# check if we have a conditional statement and, if so, only
		# print if this condition evaluates to TRUE or if the condition is
		# has the AllowFalse property - only print if FALSE
		$condition = trim($this->attributes['showif']);
		if ($condition) {
			$allowfalse = false;
			if (substr($condition,-9) == '_is_false') {
				$condition = substr($condition,0,-9);
				$allowfalse = true;
			}
			global $XTRAS_PATH;
			include_once($XTRAS_PATH.'/conditions/'.$condition.'/'.$condition.'.inc');
			# if the class doesn't exist then die
			if (class_exists($condition)) {
				$cond = new $condition($this);
				$result = $cond->evaluate($this->attributes['showif_conds']);
				if ((!$result && !$allowfalse) || ($result && $allowfalse)) {
					# conditional test failed
					return;
				}
			}
		}


		if ($this->cells) {
			echo '<tr';
			foreach($this->attributes as $name => $val) {
				if ($val == '') continue;
				if ($name == 'showif') continue;
				echo ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
			}#end foreach
			echo '>';

			for($i = 0; $i < count($this->cells); $i++) {
				$this->cells[$i]->paint(false, $keyword_replacements);
			}#end for

			echo '</tr>';
		}

	}#end paint()


	 #################################################
	# Given a map of file and page ids from old to new
	# (usually during duplication of sites or pages)
	# this will change all the links etc in the bodycopy
	# to reflect the mapping
	function remap_link_ids(&$map) {
		for($i = 0; $i < count($this->cells); $i++) {
			$this->cells[$i]->remap_link_ids($map);
		}#end for
	}


	#####################################################
	# Given a fileid, cell type checks its contents to
	# see if the file is used
	function uses_file($fileid) {
		for($i = 0; $i < count($this->cells); $i++) {
			if ($this->cells[$i]->uses_file($fileid)) return true;
		}#end for
		return false;
	}


	  ###########################################################
	 # prints the JavaScript needed by the backend functions for 
	# this table 
	function print_backend_js_data($this_bodycopy_name, $tableid, $rowid) {

		?> 
							case <?=$rowid?> :

								switch (cellid) {
									<?
									for($i = 0; $i < count($this->cells); $i++) {
										$this->cells[$i]->print_backend_js_data($this_bodycopy_name, $tableid, $rowid, $i);
									}#end for
									?>

									default :
										retVal["attributes"] = '<?=var_serialise($this->attributes, true)?>';

								}// end switch cellid

							break;
		<?
	
	}#end print_backend_js_data()


	
	/**
	* Prints the backend interface for this table row
	*
	* @param string		$this_bodycopy_name A unique name (to the template) for this bodycopy
	* @param integer	$tableid			The ID of the table this row is in
	* @param integer	$rowid				The ID of the row we are printing
	* @access public
	* @returns void
	*/
	function print_backend($this_bodycopy_name, $tableid, $rowid, $num_rows) {
		global $_BODYCOPY;
		
		?><tr><?
			if ($rowid == 0) {
				?>
				<td class="bodycopy-edit" background="" nowrap align="center" width="1%"><?
						# can't reorder if only one row
						if ($num_rows > 1) {
							print_bodycopy_icon("javascript: bodycopy_edit_table_row_order('$this_bodycopy_name', $tableid);", 'Reorder Table '.($tableid + 1).'\'s Rows ?', '', 'reorder_rows');
						} else {
							echo '&nbsp;';
						}
					?></td> 
				<?
			} else {
			?>
				<td class="bodycopy-edit" background=""><img src="images/blank.gif" width="1" height="1" border="0"></td>
			<?
			}
		# print the buttons
		for($i = 0; $i < count($this->cells); $i++) {
			$this->cells[$i]->print_backend($this_bodycopy_name, $tableid, $rowid, $i);
		}#end for
		?>
		</tr>
		<?
		$attribute_list = '';
		foreach($this->attributes as $name => $val) {
			if ($val == '') continue;
			$attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
		}#end foreach

		?> 
		<tr<?=$attribute_list?>>
			<td class="bodycopy-edit" align="center" background="" id="<?=$this_bodycopy_name.'_row_'.$tableid.'_'.$rowid?>" height="100%">
				<table border="0" cellspacing="0" cellpadding="0" height="100%">
					<tr height="1%">
						<td valign="top"><?
						print_bodycopy_icon("javascript: bodycopy_insert_table_row('$this_bodycopy_name', $tableid, $rowid, true);", 'Insert a New Row Above', '', 'insert_row_above', '', '20', '7'); 
					?>
						</td>
					</tr>
					<tr height="100%">
						<td valign="middle">
						<?
						print_bodycopy_icon("javascript: bodycopy_edit_table_row_properties('$this_bodycopy_name', $tableid, $rowid);", 'Edit Row '.($rowid + 1).'\'s Properties', (($attribute_list) ? 'Current Properties :<br>'.$attribute_list : ''), 'edit_row_properties');
						echo '<br>';
						print_bodycopy_icon("javascript: bodycopy_delete_table_row('$this_bodycopy_name', $tableid, $rowid);", 'Delete Row '.($rowid + 1).' ?', '', 'delete');
						echo '<br>';
						?>
						</td>
					</tr>
					<tr height="1%">
						<td valign="bottom">
						<?
						print_bodycopy_icon("javascript: bodycopy_insert_table_row('$this_bodycopy_name', $tableid, $rowid, false);", 'Insert a New Row Below', '', 'insert_row_below', '', '20', '7'); 
						?>
						</td>
					</tr>
				</table>
			</td> 
		<?
			# now print the cell contents
			for($i = 0; $i < count($this->cells); $i++) {
				$this->cells[$i]->paint(true);
			}#end for
		?> 
		</tr>
		<?

	}#end print_backend()

	 #########################################
	# gets the number of columns for this row
	function num_cols($add_colspans=true) {
		$num = 0;
		for($i = 0; $i < count($this->cells); $i++) {
			if ($add_colspans) {
				$num += $this->cells[$i]->colspan();
			} else {
				$num++;
			}
		}#end for
		return $num;
	}#end num_cols()


	 ###############################################
	# process the input created by print_backend()
	function update($this_bodycopy_name='bodycopy', $tableid, $rowid) {
		global $bodycopy_action, $bodycopy_name, $bodycopy_data, $bodycopy_saved; 
		global $_BODYCOPY;

		$msg = Array();

		if (count($bodycopy_saved[$this_bodycopy_name][$tableid]['rows'][$rowid]['attributes'])) {
			
			foreach($bodycopy_saved[$this_bodycopy_name][$tableid]['rows'][$rowid]['attributes'] as $name => $value) {
				if ($tmp_msg = $this->set_attribute($name, $value)) {
					$tmp_msg = 'Table '.($tableid+1).', row '.($rowid+1).": $tmp_msg";
					$msg = array_merge($msg, $tmp_msg);
				}
			}#end foreach

		} #end if


		 ############################################################################
		# only update if we are the table row that submitted the form
		if (($bodycopy_data['tableid'] == $tableid && $bodycopy_data['rowid'] == $rowid && $this_bodycopy_name == $bodycopy_name) || count($bodycopy_saved[$this_bodycopy_name][$tableid]['rows'][$rowid])) {

			$counter = 0;
			$colspan_todo = Array();

			# check to see if any cells from this row need their colspans changed
			foreach($bodycopy_saved[$this_bodycopy_name][$tableid]['rows'][$rowid]['cells'] as $cellid => $cell_data) {

				if (isset($this->cells[$cellid])) {

					$current_span = $this->cells[$cellid]->colspan();
					$colspaning   = $cell_data['attributes']['colspan'];
						
					# if the current span is '1' and the new span is '' then a cell is being created
					# so we dont want to do anything
					if ($current_span == 1 && trim($colspaning) == '') { $colspaning = $current_span; }
					
					# if the user left colspan blank, dont change anything
					if (trim($colspaning) == '') {
						$colspaning = $current_span; 
						$bodycopy_saved[$this_bodycopy_name][$tableid]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = $current_span;
					}
					
					# if the user entered a negative colspan - or '0', we can assume they want a colspan of '1'
					if (intval($colspaning) <= 0) { 
						$colspaning = 1; 
						$bodycopy_saved[$this_bodycopy_name][$tableid]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = 1;
					}
				} else {
					$colspaning = 0;
					$current_span = 0;
				}
				
				if ($colspaning != $current_span) {
					# put this colspan into a todo list that we will do later
					$colspan_todo[$counter]['cell']			= $cellid;
					$colspan_todo[$counter]['colspaning']	= $colspaning;
					$colspan_todo[$counter]['current_span'] = $current_span;
					$counter++;
				}#end if
			}


			 ################################
			# doing the colspan thing

			if (count($colspan_todo)) {

				foreach ($colspan_todo as $colspan) {

					$cellid = $colspan['cell'];
					$colspaning = $colspan['colspaning'];
					$current_span = $colspan['current_span'];
					$conflicts = false;

					# if the current span is '1' and the new span is '' then a cell is being created
					# so we dont want to do anything
					if ($current_span == 1 && trim($colspaning) == '') { $colspaning = $current_span; }
					
					# if the user left colspan blank, dont change anything
					if (trim($colspaning) == '') {
						$colspaning = $current_span; 
						$bodycopy_saved[$this_bodycopy_name][$tableid]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = $current_span;
					}
					
					# if the user entered a negative colspan - or '0', we can assume they want a colspan of '1'
					if (intval($colspaning) <= 0) { 
						$colspaning = 1; 
						$bodycopy_saved[$this_bodycopy_name][$tableid]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = 1;
					}
			
					if ($colspaning != $current_span) {
						
						# check for cell type conflicts
						$cell_type = get_class($this->cells[$cellid]->type);
						if (strstr($cell_type, 'iframe') || strstr($cell_type, 'file')) {
							# this cell has type of iframe or file/image
							# we can only colspan if the other cells are empty
							for($i = ($cellid + 1); $i <= ($cellid + ($colspaning - $current_span)); $i++) {
								if (!$this->cells[$i]->type->empty_cell()) {
									$msg[] = 'Cannot create colspan due to type conflict';
									$conflicts = true;
								}
							}
						} else {
							# we need to check the other cells for conflicts
							for($i = ($cellid + 1); $i <= ($cellid + ($colspaning - $current_span)); $i++) {
								$cell_type = get_class($this->cells[$i]->type);
								if (strstr($cell_type, 'iframe') || strstr($cell_type, 'file')) {
									# we can keep going if the cell is empty
									# otherwise die
									if (!$this->cells[$i]->type->empty_cell()) {
										$i++;
										$msg[] = 'Cannot create colspan due to type conflict - change type of cell '.$i.' or remove the cells contents';
										$conflicts = true;
									}
								}
							}
						}

						if (!$conflicts) {

							# make sure there are sufficient cells to create the colspan
							$spanable_cells = count($this->cells) - ($cellid +1);
							$needed_cells = ($colspaning - $current_span);
						
							if ($spanable_cells < $needed_cells) {
								# dont end up changing the colspan at all
								$bodycopy_saved[$this_bodycopy_name][$tableid]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = $current_span;
								$msg[] = 'Cannot create colspan - not enough cells';

							} else {
								if ($colspaning > $current_span && $spanable_cells >= 1) {
									# we are increasing the colspan
									$new_html = $this->cells[$cellid]->type->html;
															
									# check for more colspans first
									$colspan_error = 0;
									for($i = ($cellid + 1); $i <= ($cellid + ($colspaning - $current_span)); $i++) {
										if ($this->cells[$i]->attributes['colspan'] > 1) { $colspan_error = 1; }
									}
									if ($colspan_error == 0) {
										for($i = ($cellid + 1); $i <= ($cellid + ($colspaning - $current_span)); $i++) {
											$cell_html = trim($this->cells[($cellid +1)]->type->html);
											$new_html .= (trim($new_html) != '' && trim($cell_html) != '') ? '<hr>' : '';
											$new_html .= (trim($cell_html) != '') ? $cell_html : '';							
											$this->delete_cell(($cellid+1), true);
										}
										
										if (trim($new_html) != '') {
											# save the new html into the cell
											$this->cells[$cellid]->type->set_html($new_html);
										}
									} else {
										# we are trying to colspan into another colspan
										# dont end up changing the colspan at all
										$bodycopy_saved[$this_bodycopy_name][$tableid]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = $current_span;
										$msg[] = 'Cannot colspan into another colspan';
										
									}
								}
								else if ($colspaning < $current_span) {
									# decreasing the colspan
									$cell_type = get_class($this->cells[$cellid]->type);
									for($i = 0; $i < ($current_span - $colspaning); $i++) {
										$this->insert_cell($cellid, false, $cell_type, true);
									}#end for

								}#end else

							} #end else

						}#end if (no conflicts)

					}#end if

				}#end foreach

			}#end if

			###########################
			# now update the cells
			for($i = 0; $i < count($this->cells); $i++) {
				if ($tmp_msg = $this->cells[$i]->update($this_bodycopy_name, $tableid, $rowid, $i)) {
					$msg = array_merge($msg, $tmp_msg);
				}
			}#end for

		}#end if

		return $msg;

	}#end update()

	 #################################################
	# return all the keywords that are in the bodycopy
	# in the form "%keyword%"
	function get_keywords(&$keywords) {
		for($i = 0; $i < count($this->cells); $i++) {
			$this->cells[$i]->get_keywords($keywords);
		}#end for
	}

	 ################################################
	# gets all the anchors from the bodycopy
	function get_anchors(&$anchors) {
		for($i = 0; $i < count($this->cells); $i++) {
			$this->cells[$i]->get_anchors($anchors);
		}#end for
	}


}#end Class BodyCopy_Table_Row

?>
