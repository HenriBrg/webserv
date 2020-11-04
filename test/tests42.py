# ------------------------------------------------------------------ # 
# ------------------------------------------------------------------ # 
# ------------------------------------------------------------------ # 

# STAGE 1

# GET http://localhost:8080/
# POST http://localhost:8080/ with a size of 0
# HEAD http://localhost:8080/
# GET http://localhost:8080/directory return ./youpi.bad_extension
# GET http://localhost:8080/directory/youpi.bla content returned: youpi.bla
# GET Expected 404 on http://localhost:8080/directory/oulalala content returned: <html><body><h1>Error 404</h1></body></html>
GET http://localhost:8080/directory/nop returned: /nop/youpi.bad_extension
GET http://localhost:8080/directory/nop/ returned: /nop/youpi.bad_extension
GET http://localhost:8080/directory/nop/other.pouic returned: other.pouic
GET Expected 404 on http://localhost:8080/directory/nop/other.pouac content returned: <html><body><h1>Error 404</h1></body></html>
GET Expected 404 on http://localhost:8080/directory/Yeah returned: <html><body><h1>Error 404</h1></body></html>
GET http://localhost:8080/directory/Yeah/not_happy.bad_extension  content returned:
# Test Put http://localhost:8080/put_test/file_should_exist_after with a size of 1000
# content returned: Ressource created
the file file_should_exist_after should have been created with a size of 1000 bytes. Check that now
# Test Put http://localhost:8080/put_test/file_should_exist_after with a size of 10000000
# content returned:
# ==================================================================
# the file file_should_exist_after should still be there but this time with a size of 10000000 bytes. Check that now


# STAGE 2

# ------------------------------------------------------------------ # 
# ------------------------------------------------------------------ # 
# ------------------------------------------------------------------ # 


# Test POST http://localhost:8080/directory/youpi.bla with a size of 100000000

# Test POST http://localhost:8080/directory/youpla.bla with a size of 100000000

# Test POST http://localhost:8080/directory/youpi.bla with a size of 100000 with special headers

# Test POST http://localhost:8080/post_body with a size of 0

# Test POST http://localhost:8080/post_body with a size of 100

# Test POST http://localhost:8080/post_body with a size of 200

# Test POST http://localhost:8080/post_body with a size of 101

# Test multiple workers(5) doing multiple times(15): GET on /

# Test multiple workers(20) doing multiple times(5000): GET on /

# Test multiple workers(128) doing multiple times(50): GET on /directory/nop

# Test multiple workers(20) doing multiple times(5): Put on /put_test/multiple_same with size 1000000
# ==================================================================
# the file multiple_same should exists with a size of 1000000 bytes. Check that now
# ==================================================================
# press enter to continue

# Test multiple workers(20) doing multiple times(5): Post on /directory/youpi.bla with size 100000000