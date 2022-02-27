import { Blockchain } from "@ibct-dev/sclest";
import { expect } from "chai";

function seconds_since_epoch(d: number) {
    return Math.floor(d / 1000);
}

beforeEach(async () => {
    await new Promise(resolve => setTimeout(resolve, 1000));
    console.log("----------------------");
});

describe("lemonade.c v2 컨트랙트 테스트", () => {
    const debug = 1;
    const isLocal = true;
    const bc = new Blockchain({ host: "127.0.0.1", port: "8888" }); // test

    /* 테스트 변수들 */
    const manager = "lemonade.c"; // 컨트랙트 배포 계정
    const user = "test.p";
    const anotherUser = "test1.p";
    let contractTester: any;
    let ledTokenTester: any;
    let ledTester: any;
    let errorCount = 0;
    const initialize = 1000000;
    const lemInitialize = 1000000;

    describe("테스트가 시작되면, 블록체인이 연결 되어야 함", async () => {
        it("블록체인을 연결함", async () => {
            expect(await bc.checkConntection()).eq(true);
        });
    });

    if (isLocal) {
        describe("로컬블록체인이라면, 컨트랙트를 새로 배포해야 함", async () => {
            it("배포 계정을 필요한 경우 생성하고, 컨트랙트를 배포함", async () => {
                try {
                    contractTester = await bc.createAccountByCreator(
                        manager,
                        "led"
                    );
                    await contractTester.setContract("lemonade.c");
                    expect(contractTester.accountName).eq(manager);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: account is already created: ${error}`
                    );
                    errorCount += 1;
                    contractTester = await bc.getAccount(manager);
                    expect(contractTester.accountName).eq(manager);
                }
            });
            it("테스트를 위한 추가 계정들을 생성함", async () => {
                try {
                    ledTokenTester = await bc.getAccount("led.token");
                    ledTester = await bc.getAccount("led");
                    await bc.createAccountByCreator(user, "led");
                    await bc.createAccountByCreator(anotherUser, "led");
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: account is already created: ${error}`
                    );
                    errorCount += 1;
                    ledTokenTester = await bc.getAccount("led.token");
                    ledTester = await bc.getAccount("led");
                    await bc.getAccount(user);
                    await bc.getAccount(anotherUser);
                }
            });
            it("led.code 권한 생성", async () => {
                try {
                    const authData = {
                        threshold: 1,
                        keys: [
                            {
                                key: 'EOS8EReqzz88PbvNa8afvTkAhAdfbwgfRwfy4AMwS3K2thvFaMD9S',
                                weight: 1
                            }
                        ],
                        accounts: [
                            {
                                permission:
                                {
                                    actor: manager,
                                    permission: "led.code"
                                },
                                weight: 1
                            }
                        ],
                        waits: [],
                    }
                    const actionResult = await ledTester.actions.updateauth(
                        {
                            account: manager,
                            permission: "active",
                            parent: "owner",
                            auth: authData,
                        },
                        [
                            {
                                actor: manager,
                                permission: "owner",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`${user} 유저가 사용할 토큰을 지급함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: "led",
                            to: user,
                            quantity: `${initialize}.0000 LED`,
                            memo: `SHOW ME THE MONEY`
                        },
                        [
                            {
                                actor: "led",
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`${anotherUser} 유저가 사용할 토큰을 지급함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: "led",
                            to: anotherUser,
                            quantity: `${initialize}.0000 LED`,
                            memo: `SHOW ME THE MONEY`
                        },
                        [
                            {
                                actor: "led",
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`${manager} 유저가 사용할 토큰을 지급함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: "led",
                            to: manager,
                            quantity: `${initialize}.0000 LED`,
                            memo: `SHOW ME THE MONEY`
                        },
                        [
                            {
                                actor: "led",
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`${manager} 유저가 사용할 거버넌스 토큰 생성`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.create(
                        {
                            issuer: manager,
                            maximum_supply: `${lemInitialize}.0000 LEM`,
                        },
                        [
                            {
                                actor: "led.token",
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot create: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`거버넌스 토큰 초기 생성`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.issue(
                        {
                            to: manager,
                            quantity: `400.0000 LEM`,
                            memo: `SHOW ME THE MONEY`
                        },
                        [
                            {
                                actor: manager,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot issue: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`${user} 유저가 사용할 거버넌스 토큰을 지급함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: manager,
                            to: user,
                            quantity: `400.0000 LEM`,
                            memo: `SHOW ME THE MONEY`
                        },
                        [
                            {
                                actor: manager,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it("led.code 권한 생성", async () => {
                try {
                    const authData = {
                        threshold: 1,
                        keys: [
                            {
                                key: 'EOS8EReqzz88PbvNa8afvTkAhAdfbwgfRwfy4AMwS3K2thvFaMD9S',
                                weight: 1
                            }
                        ],
                        accounts: [
                            {
                                permission:
                                {
                                    actor: manager,
                                    permission: "led.code"
                                },
                                weight: 1
                            }
                        ],
                        waits: [],
                    }
                    const actionResult = await ledTester.actions.updateauth(
                        {
                            account: manager,
                            permission: "active",
                            parent: "owner",
                            auth: authData,
                        },
                        [
                            {
                                actor: manager,
                                permission: "owner",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
        });

        describe("openext(): 토큰을 저장할 채널 생성", async () => {
            it(`${user}가 LED 토큰을 저장할 채널을 만듬 `, async () => {
                try {
                    const actionResult = await contractTester.actions.openext(
                        {
                            user: user,
                            ext_symbol: {
                                sym: "4,LED",
                                contract: "led.token"
                            }
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot openext: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`dexacnts 테이블에 생성 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[0];
                expect(onlyCreated).to.not.undefined;
            });

            it(`${user}가 LEM 토큰을 저장할 채널을 만듬 `, async () => {
                try {
                    const actionResult = await contractTester.actions.openext(
                        {
                            user: user,
                            ext_symbol: {
                                sym: "4,LEM",
                                contract: "led.token"
                            }
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot openext: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`dexacnts 테이블에 생성 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[1];
                expect(onlyCreated).to.not.undefined;
            });

            it(`${user}가 더미 토큰을 저장할 채널을 만듬 `, async () => {
                try {
                    const actionResult = await contractTester.actions.openext(
                        {
                            user: user,
                            ext_symbol: {
                                sym: "4,DUMMY",
                                contract: "led.token"
                            }
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot openext: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`dexacnts 테이블에 생성 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[2];
                expect(onlyCreated).to.not.undefined;
            });
        });

        describe("closeext(): 토큰을 저장할 채널 생성", async () => {
            it(`${user}가 더미 토큰을 저장할 채널을 지움 `, async () => {
                try {
                    const actionResult = await contractTester.actions.closeext(
                        {
                            user: user,
                            to: user,
                            ext_symbol: {
                                sym: "4,DUMMY",
                                contract: "led.token"
                            },
                            memo: ""
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot closeext: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`dexacnts 테이블에 제거 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[2];
                expect(onlyCreated).to.undefined;
            });
        });

        describe("deposit() & withdraw(): 채널에 토큰을 저장하고 꺼내기", async () => {
            it(`${user}가 LEM을 저장함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: user,
                            to: manager,
                            quantity: `300.0000 LEM`,
                            memo: `deposit`
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`dexacnts 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[1];
            });
            it(`${user}가 LEM을 꺼냄`, async () => {
                try {
                    const actionResult = await contractTester.actions.withdraw(
                        {
                            user: user,
                            to: user,
                            to_withdraw: {
                                quantity: "100.0000 LEM",
                                contract: "led.token"
                            },
                            memo: `out`
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot withdraw: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`dexacnts 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.dexacnts({
                    scope: user,
                });
                if (debug) console.log(`Dexacnts\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[1];
            });
        });
    }
});
