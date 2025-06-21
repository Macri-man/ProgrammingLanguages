echo "Hello, $name"    # expands $name


echo 'Hello, $name'    # prints literally: Hello, $name

echo ${var:-"default"}   # prints var if set, otherwise "default"

: ${var:="default"}   # assigns "default" to var if unset/null
