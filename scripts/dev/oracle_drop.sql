spool drop.log
set serveroutput on

prompt drop all tables in the schema

begin

  for rec in (select TABLE_NAME, constraint_name from user_constraints where constraint_type = 'R') LOOP
    execute immediate 'alter table '||rec.table_name ||' drop constraint '||rec.constraint_name || ';';
  end loop;

  for rec in (select view_name from user_views WHERE view_name LIKE 'SQ_%') loop
       execute immediate 'drop view &&1..'||rec.view_name;
  end loop;

  for rec in (select table_name from user_tables WHERE table_name LIKE 'SQ_%') loop
       execute immediate 'drop table &&1..'||rec.table_name;
  end loop;

  for rec in (select sequence_name from user_sequences WHERE sequence_name LIKE 'SQ_%') loop
      execute immediate 'drop sequence &&1..'||rec.sequence_name;
  end loop;

  for rec in (select trigger_name from user_triggers WHERE trigger_name LIKE 'SQ_%') loop
      execute immediate 'drop trigger &&1..'||rec.trigger_name;
  end loop;

  for rec in (select object_name from user_objects where object_type='FUNCTION' AND (object_name LIKE 'sq_%' OR object_name LIKE 'SQ_%' OR object_name LIKE 'ASSET%')) loop
      execute immediate 'drop function &&1..' || rec.object_name;
  end loop;

  execute immediate 'drop package &&1..sq_search_pkg';
  execute immediate 'drop package &&1..sq_common_pkg';


  execute immediate 'drop type &&1..VARCHAR_500_TABLE';
  execute immediate 'drop type &&1..VARCHAR_2000_TABLE';
end;
/
exit;
/
